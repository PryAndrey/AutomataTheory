class Program
{
    static void Main(string[] args)
    {
        if (args.Length != 2 && args.Length != 3)
        {
            Console.Error.WriteLine("Usage: program.exe <input_file> <output_file>");
            return;
        }

        string command;
        string inputFile;
        string outputFile;

        if (args.Length == 3)
        {
            command = args[0];
            inputFile = args[1];
            outputFile = args[2];
        }
        else
        {
            command = AppDomain.CurrentDomain.FriendlyName;
            inputFile = args[0];
            outputFile = args[1];
        }

        try
        {
            var gr = new GrammarReader();
            gr.ReadFile(inputFile);
            gr.WriteToFile(outputFile);
        }
        catch (Exception e)
        {
            Console.WriteLine(e.Message);
        }
    }
}
