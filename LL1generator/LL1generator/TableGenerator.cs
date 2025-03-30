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

    public static List<string> ExtractTokens(string input)
    {
        List<string> tokens = new List<string>();

        string pattern = $@"<[^>]+>|[\wа-яА-Яε=+*,()]+|ε|⟂";

        Regex regex = new Regex(pattern);
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
            var token = ExtractTokens(match.Replace(" ", ""));
            if (token.Count > 0)
            {
                parsedParams.Add(token);
            }
        }

        return parsedParams;
    }

    public void ReadGrammarRules(List<KeyValuePair<string, string>> grammarVector)
    {
        Dictionary<string, int> statesNamesToIndex = new Dictionary<string, int>();
        // Указывает на следующую позицию (для ε)
        Dictionary<string, HashSet<int>> afterStatesPos = new Dictionary<string, HashSet<int>>();
        // При изменении напр. мн-ва соответствующие состояния тоже должны измениться 
        Dictionary<string, HashSet<int>> statesInfluence = new Dictionary<string, HashSet<int>>();
        // Содержит все развилки => суммируем в напр. мн-вах ссылающихся состояний
        Dictionary<string, HashSet<int>> statesInfluenceSummary = new Dictionary<string, HashSet<int>>();

        foreach (var pair in grammarVector)
        {
            string grammarKey = pair.Key;
            string grammarTransition = pair.Value;

            var parsedList = ParseGrammarTransition(grammarTransition);
            int nextPosition = _states.Count + parsedList.Count;
            statesNamesToIndex.Add(grammarKey, _states.Count);
            for (int i = 0; i < parsedList.Count; i++)
            {
                var parsedParamsList = parsedList[i];
                _states.Add(new TableRow(_states.Count, grammarKey, nextPosition, new HashSet<string>(),
                    i == parsedList.Count - 1, false, false, false));
                nextPosition += parsedParamsList.Count;

                string token = parsedParamsList[0];
                bool isNotTerm = token.Contains("<");
                if (isNotTerm)
                {
                    if (!statesInfluence.ContainsKey(token))
                    {
                        statesInfluence.Add(token, new HashSet<int> { _states.Count - 1 });
                    }
                    else
                    {
                        statesInfluence[token].Add(_states.Count - 1);
                    }
                }

                if (!statesInfluenceSummary.ContainsKey(grammarKey))
                {
                    statesInfluenceSummary.Add(grammarKey, new HashSet<int> { _states.Count - 1 });
                }
                else
                {
                    statesInfluenceSummary[grammarKey].Add(_states.Count - 1);
                }
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
                        : new HashSet<string>();
                    if (j < parsedParamsList.Count() - 1)
                    {
                        if (!afterStatesPos.ContainsKey(token))
                        {
                            afterStatesPos.Add(token, new HashSet<int> { _states.Count + 1 });
                        }
                        else
                        {
                            afterStatesPos[token].Add(_states.Count + 1);
                        }
                    }

                    _states.Add(new TableRow(_states.Count, token, to, nextSymbolSet, error, shift, toStack, end));
                }
            }
        }

        // Заполнение ссылок
        foreach (var state in _states)
        {
            if (state.To == -2)
            {
                state.To = statesNamesToIndex[state.Name];
            }
        }

        // Hashset
        foreach (var state in _states)
        {
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
                    $"{state.Id};{state.Name};{state.To};{string.Join(",", state.NextSymbolSet)};{state.Shift};{state.Error};{state.ToStack};{state.End}");
            }

            file.WriteLine();
        }
    }

    private static readonly string N_TERM = $@"<[\wа-яА-Я]+>";
    private static readonly string TERM = $@"(?:[\wа-яА-Я⟂ε=+*,()]|ε)";
    private static readonly string ANY = $@"(?:{N_TERM}|{TERM}|{OR})";
    private const string TO = @"\s*->\s*";
    private const string OR = @"\s*\|\s*";

    private string GRAMMAR_RULES = $@"^\s*({N_TERM}){TO}((?:{ANY}\s*)*)\s*$";
}