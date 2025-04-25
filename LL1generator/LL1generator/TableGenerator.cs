using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;

public class GrammarReader
{
    public class TableRow
    {
        public int Id { get; set; }
        public string Name { get; set; }
        public int To { get; set; }
        public HashSet<string> NextSymbolSet { get; set; }
        public bool Error { get; set; }
        public bool Shift { get; set; }
        public bool ToStack { get; set; }
        public bool End { get; set; }

        public TableRow(int id, string name, int to,
            HashSet<string> nextSymbolSet,
            bool error, bool shift,
            bool toStack, bool end)
        {
            Id = id;
            Name = name;
            To = to;
            NextSymbolSet = nextSymbolSet;
            Error = error;
            Shift = shift;
            ToStack = toStack;
            End = end;
        }
    }

    private List<TableRow> _states = new List<TableRow>();
    private Dictionary<string, HashSet<string>> _firstSets = new Dictionary<string, HashSet<string>>();
    private Dictionary<string, HashSet<string>> _followSets = new Dictionary<string, HashSet<string>>();
    private Dictionary<string, List<List<string>>> _statesRules = new Dictionary<string, List<List<string>>>();
    private Dictionary<string, int> _statesNamesToIndex = new Dictionary<string, int>();

    public static List<string> ExtractTokens(string input)
    {
        List<string> tokens = new List<string>();

        Regex regex = new Regex(Pattern);
        MatchCollection matches = regex.Matches(input);

        foreach (Match match in matches)
        {
            tokens.Add(match.Value);
        }

        return tokens;
    }

    public List<List<string>> ParseGrammarTransition(string grammarTransition)
    {
        var matches = grammarTransition.Split("|");

        var parsedParams = new List<List<string>>();

        foreach (string match in matches)
        {
            var token = ExtractTokens(match);
            if (token.Count > 0)
            {
                parsedParams.Add(token);
            }
        }

        return parsedParams;
    }

    public void ReadGrammarRules(List<KeyValuePair<string, string>> grammarVector)
    {
        Dictionary<string, HashSet<string>> combinedSets = new Dictionary<string, HashSet<string>>();
        foreach (var pair in grammarVector)
        {
            _statesRules[pair.Key] = ParseGrammarTransition(pair.Value);
        }

        ComputeFirstSets(_statesRules);
        ComputeFollowSets(_statesRules);

        foreach (var rule in _statesRules)
        {
            string grammarKey = rule.Key;
            List<List<string>> parsedList = rule.Value;

            int nextPosition = _states.Count + parsedList.Count;
            _statesNamesToIndex.Add(grammarKey, _states.Count);
            for (int i = 0; i < parsedList.Count; i++)
            {
                var parsedParamsList = parsedList[i];

                var firstOfAlt = ComputeFirstForSequence1(parsedParamsList);

                // Если альтернатива может порождать ε, добавляем FOLLOW(nonTerminal)
                if (CanDeriveEpsilon(parsedParamsList))
                {
                    firstOfAlt.UnionWith(_followSets[grammarKey]);
                    firstOfAlt.Remove("ε"); // ε не включается в DIRECT
                }

                if (!combinedSets.ContainsKey(grammarKey))
                {
                    combinedSets.Add(grammarKey, _firstSets[grammarKey]);
                }
                else
                {
                    combinedSets[grammarKey].UnionWith(_firstSets[grammarKey]);
                }

                _states.Add(new TableRow(_states.Count, grammarKey, nextPosition, firstOfAlt,
                    i == parsedList.Count - 1, false, false, false));
                nextPosition += parsedParamsList.Count;
            }

            for (int i = 0; i < parsedList.Count; i++)
            {
                var parsedParamsList = parsedList[i];

                for (int j = 0; j < parsedParamsList.Count(); j++)
                {
                    string token = parsedParamsList[j];

                    bool isNotTerm = token.Contains("<");
                    bool error = true;
                    bool shift = !(isNotTerm || token == "ε");
                    bool toStack = isNotTerm && j < parsedParamsList.Count() - 1;
                    bool end = token == "⟂";
                    int to =
                        j < parsedParamsList.Count() - 1 &&
                        !isNotTerm &&
                        token != "ε" &&
                        token != "⟂"
                            ? _states.Count + 1 // терминал указывает на следующий элемент
                            : (token == "ε" || j == parsedParamsList.Count() - 1) && !isNotTerm
                                ? -1 // null если терминал в конце
                                : -2; // нужно переопределить ссылку
                    HashSet<string> nextSymbolSet = (!isNotTerm || token == "⟂") && token != "ε"
                        ? new HashSet<string>([token])
                        : token == "ε"
                            ? _followSets[grammarKey]
                            : new HashSet<string>();

                    _states.Add(new TableRow(_states.Count, token, to, nextSymbolSet, error, shift, toStack, end));
                }
            }
        }

        // Заполнение ссылок
        foreach (var state in _states)
        {
            if (state.To == -2)
            {
                state.To = _statesNamesToIndex[state.Name];
            }

            if (state.NextSymbolSet.Count == 0)
            {
                state.NextSymbolSet = combinedSets[state.Name];
            }

            if (state.NextSymbolSet.Contains("ε"))
            {
                state.NextSymbolSet.UnionWith(_followSets[state.Name]);
                state.NextSymbolSet.Remove("ε");
            }
        }
    }

    private bool CanDeriveEpsilon(List<string> sequence)
    {
        foreach (var symbol in sequence)
        {
            if (!symbol.Contains("<"))
                return false;

            if (!_firstSets[symbol].Contains("ε"))
                return false;
        }

        return true;
    }

    private void ComputeFirstSets(Dictionary<string, List<List<string>>> statesRules)
    {
        // Инициализация FIRST для всех нетерминалов
        foreach (var nonTerminal in statesRules.Keys)
        {
            _firstSets[nonTerminal] = new HashSet<string>();
        }

        bool changed;
        do
        {
            changed = false;
            foreach (var rule in statesRules)
            {
                var nonTerminal = rule.Key;
                foreach (var production in rule.Value)
                {
                    var oldCount = _firstSets[nonTerminal].Count;
                    ComputeFirstForSequence(production, _firstSets[nonTerminal]);
                    if (_firstSets[nonTerminal].Count > oldCount)
                    {
                        changed = true;
                    }
                }
            }
        } while (changed);
    }

    private void ComputeFirstForSequence(List<string> sequence, HashSet<string> result)
    {
        bool allCanDeriveEpsilon = true;
        foreach (var symbol in sequence)
        {
            if (!symbol.Contains("<"))
            {
                // Для терминала просто добавляем его самого
                result.Add(symbol);
                allCanDeriveEpsilon = false;
                break;
            }
            else
            {
                // Добавляем FIRST(symbol) кроме ε
                if (_firstSets.ContainsKey(symbol))
                {
                    result.UnionWith(_firstSets[symbol].Where(s => s != "ε"));

                    // Если текущий символ не может порождать ε, прерываем цепочку
                    if (!_firstSets[symbol].Contains("ε"))
                    {
                        allCanDeriveEpsilon = false;
                        break;
                    }
                }
                else
                {
                    // На случай, если символ не определен (должен быть терминалом)
                    result.Add(symbol);
                    allCanDeriveEpsilon = false;
                    break;
                }
            }
        }

        // Если вся последовательность может порождать ε, добавляем ε
        if (allCanDeriveEpsilon)
        {
            result.Add("ε");
        }
    }

    private HashSet<string> ComputeFirstForSequence1(List<string> sequence)
    {
        var first = new HashSet<string>();
        bool canDeriveEpsilon = true;

        foreach (var symbol in sequence)
        {
            if (!symbol.Contains("<"))
            {
                first.Add(symbol);
                canDeriveEpsilon = false;
                break;
            }
            else
            {
                // Добавляем FIRST(symbol) кроме ε
                foreach (var terminal in _firstSets[symbol])
                {
                    if (terminal != "ε")
                        first.Add(terminal);
                }

                // Если текущий символ не может порождать ε, выходим
                if (!_firstSets[symbol].Contains("ε"))
                {
                    canDeriveEpsilon = false;
                    break;
                }
            }
        }

        // Если вся последовательность может порождать ε, добавляем ε
        if (canDeriveEpsilon)
            first.Add("ε");

        return first;
    }

    private void ComputeFollowSets(Dictionary<string, List<List<string>>> statesRules)
    {
        // Инициализация FOLLOW для всех нетерминалов
        foreach (var nonTerminal in statesRules.Keys)
        {
            _followSets[nonTerminal] = new HashSet<string>();
        }

        _followSets[statesRules.First().Key].Add("$");

        bool changed;
        do
        {
            changed = false;
            foreach (var rule in statesRules)
            {
                var nonTerminal = rule.Key;
                foreach (var production in rule.Value)
                {
                    for (int i = 0; i < production.Count; i++)
                    {
                        var currentSymbol = production[i];
                        if (!currentSymbol.Contains("<")) continue;

                        var oldCount = _followSets[currentSymbol].Count;

                        // Случай 1: A → αBβ - добавляем FIRST(β) кроме ε
                        if (i < production.Count - 1)
                        {
                            var beta = production.Skip(i + 1).ToList();
                            var firstBeta = new HashSet<string>();
                            ComputeFirstForSequence(beta, firstBeta);

                            _followSets[currentSymbol].UnionWith(firstBeta.Where(s => s != "ε"));

                            // Если β ⇒* ε, добавляем FOLLOW(A)
                            if (firstBeta.Contains("ε"))
                            {
                                _followSets[currentSymbol].UnionWith(_followSets[nonTerminal]);
                            }
                        }
                        // Случай 2: A → αB - добавляем FOLLOW(A)
                        else
                        {
                            _followSets[currentSymbol].UnionWith(_followSets[nonTerminal]);
                        }

                        if (_followSets[currentSymbol].Count > oldCount)
                        {
                            changed = true;
                        }
                    }
                }
            }
        } while (changed);
    }

    public void RegexRead(List<KeyValuePair<string, string>> grammarVector, string regularExpression)
    {
        Match match;
        if ((match = Regex.Match(regularExpression, GRAMMAR_RULES)).Success)
        {
            grammarVector.Add(new KeyValuePair<string, string>(match.Groups[1].Value, match.Groups[2].Value));
            return;
        }

        throw new InvalidOperationException("Wrong input format");
    }

    public void ReadFile(string fileName)
    {
        if (!File.Exists(fileName))
        {
            throw new FileNotFoundException("Could not open file: " + fileName);
        }

        var grammarVector = new List<KeyValuePair<string, string>>();
        string regularExpression = string.Empty;

        foreach (var line in File.ReadLines(fileName))
        {
            if (line.Length == 0)
            {
                continue;
            }

            if (!line.Contains("->"))
            {
                regularExpression += line;
                continue;
            }

            if (string.IsNullOrEmpty(regularExpression))
            {
                regularExpression = line;
                continue;
            }

            regularExpression = Regex.Replace(regularExpression, @"\s+", " ");
            RegexRead(grammarVector, regularExpression);
            regularExpression = line;
        }

        regularExpression = Regex.Replace(regularExpression, @"\s+", " ");
        RegexRead(grammarVector, regularExpression);
        ReadGrammarRules(grammarVector);
    }

    public void WriteMooreToFile(string fileName)
    {
        using (var file = new StreamWriter(fileName))
        {
            file.WriteLine("Id;Name;To;Set;Shift;Error;Stack;End");

            foreach (var state in _states)
            {
                file.WriteLine(
                    $"{state.Id};{state.Name};{state.To};{string.Join(",", ToBrackets(state.NextSymbolSet))};{state.Shift};{state.Error};{state.ToStack};{state.End}");
            }

            file.WriteLine();
        }
    }

    private HashSet<string> ToBrackets(HashSet<string> NextSymbolSet)
    {
        HashSet<string> NextSymbolSetNew = new HashSet<string>();
        foreach (var sym in NextSymbolSet)
        {
            NextSymbolSetNew.Add(sym != "," ? sym : "\',\'");
        }

        return NextSymbolSetNew;
    }

    private static readonly string N_TERM = $@"<[\wа-яА-Я'`]+>";
    private static readonly string TERM = $@"(?:[\wа-яА-Я⟂'`=+*,()]|ε)";
    private static readonly string Pattern = $@"<[^>]+>|[\wа-яА-Я'`=+*,()]+|ε|⟂";
    private static readonly string ANY = $@"(?:{N_TERM}|{TERM}|{OR})";
    private const string TO = @"\s*->\s*";
    private const string OR = @"\s*\|\s*";

    private string GRAMMAR_RULES = $@"^\s*({N_TERM}){TO}((?:{ANY}\s*)*)\s*$";
}