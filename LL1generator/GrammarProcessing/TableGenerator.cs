using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;

// todo check Неоднозначность ~ ok(Не уверен, но тесты прошли)
// todo check Продуктивность(конечность)
// todo check Недостижимость(не связанные состояния) - ok

public class GrammarReader
{
    private Dictionary<string, HashSet<string>> _firstSets = new Dictionary<string, HashSet<string>>();
    private Dictionary<string, List<List<string>>> _statesRules = new Dictionary<string, List<List<string>>>();

    private Dictionary<string, Dictionary<string, HashSet<string>>> _firstSources =
        new Dictionary<string, Dictionary<string, HashSet<string>>>();

    private Dictionary<string, bool> _ruleEndless = new Dictionary<string, bool>();
    private string _firstRuleName;
    private HashSet<string> _ruleName = new HashSet<string>();
    private HashSet<string> _callingRuleName = new HashSet<string>();

    public List<string> ExtractTokens(string input)
    {
        List<string> tokens = new List<string>();

        Regex regex = new Regex(PATTERN);
        MatchCollection matches = regex.Matches(input);

        foreach (Match match in matches)
        {
            tokens.Add(match.Value);
            if (match.Value.Contains("<"))
            {
                _callingRuleName.Add(match.Value);
            }
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
        bool isFirst = true;
        foreach (var pair in grammarVector)
        {
            if (isFirst)
            {
                isFirst = false;
                _firstRuleName = pair.Key;
            }

            _ruleName.Add(pair.Key);
            _ruleEndless.Add(pair.Key, true);
            _statesRules[pair.Key] = ParseGrammarTransition(pair.Value);
        }

        ComputeFirstSets(_statesRules);
        CheckForInfiniteGrammar();

        // С вызовом 1 правила или без него
        if (!(_ruleName.Count == _callingRuleName.Count ||
              (_ruleName.Count - 1 == _callingRuleName.Count && !_callingRuleName.Contains(_firstRuleName))))
        {
            throw new Exception("Есть лишние не терминалы");
        }
    }

    private void ComputeFirstSets(Dictionary<string, List<List<string>>> statesRules)
    {
        // Инициализация структур
        foreach (var nonTerminal in statesRules.Keys)
        {
            _firstSets[nonTerminal] = new HashSet<string>();
            _firstSources[nonTerminal] = new Dictionary<string, HashSet<string>>();
        }

        bool changed;
        do
        {
            changed = false;
            foreach (var rule in statesRules)
            {
                var nonTerminal = rule.Key;
                int productionIndex = 0;
                foreach (var production in rule.Value)
                {
                    var oldCount = _firstSets[nonTerminal].Count;
                    // Передаем уникальный идентификатор продукции (правила + индекс альтернативы)
                    string productionId = $"{nonTerminal}_{productionIndex}";
                    ComputeFirstForSequence(production, nonTerminal, productionId);
                    if (_firstSets[nonTerminal].Count > oldCount)
                    {
                        changed = true;
                    }

                    productionIndex++;
                }
            }
        } while (changed);
    }

    private void ComputeFirstForSequence(List<string> sequence, string nonTerminal, string productionId)
    {
        // Инициализация структур для хранения источников FIRST-символов
        if (!_firstSources.ContainsKey(nonTerminal))
        {
            _firstSources[nonTerminal] = new Dictionary<string, HashSet<string>>();
        }

        // Создаем уникальный идентификатор для этой продукции (альтернативы)
        string sourceId = productionId; // Можно использовать хеш последовательности или порядковый номер

        bool allCanDeriveEpsilon = true;
        foreach (var symbol in sequence)
        {
            if (!symbol.Contains("<"))
            {
                // Для терминала проверяем конфликты между разными альтернативами
                if (_firstSources[nonTerminal].ContainsKey(symbol) &&
                    _firstSources[nonTerminal][symbol].Any(s => s != sourceId))
                {
                    throw new Exception($"Неоднозначность с {symbol} в {nonTerminal} (из разных альтернатив)");
                }

                // Добавляем символ с указанием источника
                if (!_firstSources[nonTerminal].ContainsKey(symbol))
                {
                    _firstSources[nonTerminal][symbol] = new HashSet<string>();
                }

                _firstSources[nonTerminal][symbol].Add(sourceId);

                _firstSets[nonTerminal].Add(symbol);
                allCanDeriveEpsilon = false;
                break;
            }
            else
            {
                // Добавляем FIRST(symbol) кроме ε
                if (_firstSets.ContainsKey(symbol))
                {
                    foreach (var firstSymbol in _firstSets[symbol].Where(s => s != "ε"))
                    {
                        // Проверяем конфликты между разными альтернативами
                        if (_firstSources[nonTerminal].ContainsKey(firstSymbol) &&
                            _firstSources[nonTerminal][firstSymbol].Any(s => s != sourceId))
                        {
                            throw new Exception(
                                $"Неоднозначность с {firstSymbol} в {nonTerminal} (из разных альтернатив)");
                        }

                        // Добавляем символ с указанием источника
                        if (!_firstSources[nonTerminal].ContainsKey(firstSymbol))
                        {
                            _firstSources[nonTerminal][firstSymbol] = new HashSet<string>();
                        }

                        _firstSources[nonTerminal][firstSymbol].Add(sourceId);
                    }

                    _firstSets[nonTerminal].UnionWith(_firstSets[symbol].Where(s => s != "ε"));

                    if (!_firstSets[symbol].Contains("ε"))
                    {
                        allCanDeriveEpsilon = false;
                        break;
                    }
                }
                else
                {
                    // На случай, если символ не определен
                    if (_firstSources[nonTerminal].ContainsKey(symbol) &&
                        _firstSources[nonTerminal][symbol].Any(s => s != sourceId))
                    {
                        throw new Exception($"Неоднозначность с {symbol} в {nonTerminal} (из разных альтернатив)");
                    }

                    if (!_firstSources[nonTerminal].ContainsKey(symbol))
                    {
                        _firstSources[nonTerminal][symbol] = new HashSet<string>();
                    }

                    _firstSources[nonTerminal][symbol].Add(sourceId);

                    _firstSets[nonTerminal].Add(symbol);
                    allCanDeriveEpsilon = false;
                    break;
                }
            }
        }

        if (allCanDeriveEpsilon)
        {
            _firstSets[nonTerminal].Add("ε");
        }
    }

    private void CheckForInfiniteGrammar()
    {
        bool changed;
        do
        {
            changed = false;
            foreach (var rule in _statesRules)
            {
                var nonTerminal = rule.Key;
                foreach (var production in rule.Value)
                {
                    // Если продукция содержит только терминалы, то нетерминал продуктивен
                    if (production.All(symbol => !symbol.Contains("<")))
                    {
                        if (_ruleEndless[nonTerminal])
                        {
                            _ruleEndless[nonTerminal] = false;
                            changed = true;
                        }

                        continue;
                    }

                    // Если продукция содержит нетерминалы, проверяем их продуктивность
                    if (production.Any(symbol => symbol.Contains("<") && _ruleEndless[symbol]))
                    {
                        continue;
                    }

                    // Если все нетерминалы в продукции непродуктивны, то и текущий нетерминал непродуктивен
                    if (_ruleEndless[nonTerminal])
                    {
                        _ruleEndless[nonTerminal] = false;
                        changed = true;
                    }
                }
            }
        } while (changed);

        // Проверяем, есть ли нетерминалы, которые могут порождать бесконечные цепочки
        var result = "";
        foreach (var rule in _ruleEndless)
        {
            if (rule.Value)
            {
                result += rule.Key + ", ";
            }
        }

        if (result.Length != 0)
        {
            throw new Exception(
                $"Грамматика может быть бесконечной: нетерминалы {result.Remove(result.Length - 2)} могут порождать бесконечную цепочку");
        }
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

    public void WriteToFile(string fileName)
    {
        using (var file = new StreamWriter(fileName))
        {
            file.WriteLine("Id;Name;To;Set;Shift;Error;Stack;End");

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
    private static readonly string PATTERN = $@"<[^>]+>|[\wа-яА-Я'`=+*,()]+|ε|⟂";
    private static readonly string ANY = $@"(?:{N_TERM}|{TERM}|{OR})";
    private const string TO = @"\s*->\s*";
    private const string OR = @"\s*\|\s*";

    private string GRAMMAR_RULES = $@"^\s*({N_TERM}){TO}((?:{ANY}\s*)*)\s*$";
}