/* -----------------------------------------------------------------------
 * <copyright file="Program.cs" company="Hybrid Software Helix Ltd">
 *  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

using System;
using JawsMako;

namespace TextTransformExample
{
    class CustomTextTransform
    {
        static int Main(string[] args)
        {
            if (args.Length != 3)
            {
                Console.Error.WriteLine($"Usage: {AppDomain.CurrentDomain.FriendlyName} <input> <output> <inkValue>");
                return 1;
            }

            string testFilePath = "..\\..\\..\\..\\TestFiles\\";
            string inputFile = args[0];
            string outputFile = args[1];
            float inkValue = float.Parse(args[2]);

            try
            {
                using var jawsMako = IJawsMako.create();
                IJawsMako.enableAllFeatures(jawsMako);

                var implementation = new TextTransformImplementation(jawsMako, inkValue);

                var textModifier = ICustomTransform.create(
                    jawsMako,
                    implementation
                );

                var assembly = IInput.create(jawsMako, eFileFormat.eFFPDF).open(testFilePath + inputFile);
                var document = assembly.getDocument();

                uint pageCount = document.getNumPages();
                Console.WriteLine($"Processing {pageCount} pages...");

                for (uint i = 0; i < pageCount; i++)
                {
                    var page = document.getPage(i);
                    textModifier.transformPage(page);
                    page.Dispose();
                }

                Console.WriteLine("Writing output file ...");
                using var pdfOutput = IPDFOutput.create(jawsMako);
                pdfOutput.writeAssembly(assembly, outputFile);
            }
            catch (MakoException e)
            {
                string errorFormatString = jawsmakoIF_csharp.getEDLErrorString(e.m_errorCode);
                Console.Error.WriteLine($"Exception thrown: {e.m_msg}");
                return (int)e.m_errorCode;
            }
            catch (Exception e)
            {
                Console.Error.WriteLine($"std::exception thrown: {e.Message}");
                return 1;
            }

            return 0;
        }
    }
}