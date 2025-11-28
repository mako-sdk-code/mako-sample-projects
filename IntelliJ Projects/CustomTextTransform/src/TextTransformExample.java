/* -----------------------------------------------------------------------
 * <copyright file="TextTransformExample.java" company="Hybrid Software Helix Ltd">
 *  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

import com.globalgraphics.JawsMako.jawsmakoIF.*;

public class TextTransformExample
{
    public static void main(String[] args)
    {
        if (args.length != 3)
        {
            System.err.printf("Usage: java TextTransformExample <input> <output> <inkValue>%n");
            System.exit(1);
        }

        String testFilePath = "TestFiles/";
        String inputFile = args[0];
        String outputFile = args[1];
        float inkValue = Float.parseFloat(args[2]);

        try
        {
            IJawsMako jawsMako = IJawsMako.create();
            IJawsMako.enableAllFeatures(jawsMako);

            TextTransformImplementation implementation = new TextTransformImplementation(jawsMako, inkValue);

            ICustomTransform textModifier = ICustomTransform.create(jawsMako, implementation);

            IInput input = IInput.create(jawsMako, eFileFormat.eFFPDF);
            IDocumentAssembly assembly = input.open(testFilePath + inputFile);
            IDocument document = assembly.getDocument();

            long pageCount = document.getNumPages();
            System.out.printf("Processing %d pages...%n", pageCount);

            for (long i = 0; i < pageCount; i++)
            {
                IPage page = document.getPage(i);
                textModifier.transformPage(page);
                page.release();
            }

            System.out.println("Writing output file ...");
            IPDFOutput pdfOutput = IPDFOutput.create(jawsMako);
            pdfOutput.writeAssembly(assembly, outputFile);
        }
        catch (Exception e)
        {
            System.err.printf("std::exception thrown: %s%n", e.getMessage());
            System.exit(1);
        }

        System.exit(0);
    }
}
