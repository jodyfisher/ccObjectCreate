using System;
using System.IO;
using System.Linq;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;

class Program
{
    static int Main(string[] args)
    {
        string directory = args[0];
        string bundle = File.ReadAllText(Path.Combine(directory, "Sample.cs"));
        int marker = bundle.IndexOf("/// SQL Server Version", StringComparison.Ordinal);
        string bo = bundle.Substring(0, bundle.IndexOf("public class SampleD", StringComparison.Ordinal));
        string stubs = File.ReadAllText(Path.Combine(AppContext.BaseDirectory, "OutputStubs.txt"));
        var references = ((string)AppContext.GetData("TRUSTED_PLATFORM_ASSEMBLIES")).Split(Path.PathSeparator)
            .Select(path => MetadataReference.CreateFromFile(path));
        int errors = 0;
        foreach (string source in new[] { bundle.Substring(0, marker), bo + bundle.Substring(marker) })
        {
            var compilation = CSharpCompilation.Create("GeneratedChecks",
                new[] { CSharpSyntaxTree.ParseText(source), CSharpSyntaxTree.ParseText(stubs) }, references,
                new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary));
            foreach (var error in compilation.GetDiagnostics().Where(d => d.Severity == DiagnosticSeverity.Error))
            {
                Console.Error.WriteLine(error);
                errors++;
            }
        }
        // Form output contains member snippets for different forms, not complete designer classes.
        string form = "class frmSample {\n" + File.ReadAllText(Path.Combine(directory, "Sample.frm.cs")) + "\n}";
        foreach (var error in CSharpSyntaxTree.ParseText(form).GetDiagnostics().Where(d => d.Severity == DiagnosticSeverity.Error))
        {
            Console.Error.WriteLine(error);
            errors++;
        }
        Console.WriteLine(errors == 0 ? "Generated MySQL/SQL Server C# compiles with stubs; form syntax passed." : errors + " errors.");
        return errors == 0 ? 0 : 1;
    }
}
