/* --------------------------------------------------------------------------------
 *  <copyright file="SaveWithOutputIntent.cs" company="Hybrid Software Helix Ltd">
 *    Copyright (c) 2025 Hybrid Software Helix Ltd. All rights reserved.
 *  </copyright>
 *  <summary>
 *    This example is provided on an "as is" basis and without warranty of any kind.
 *    Hybrid Software Helix Ltd. does not warrant or make any representations
 *    regarding the use or results of use of this example.
 *  </summary>
 * ---------------------------------------------------------------------------------
 */

using JawsMako;
using System;
using static JawsMako.jawsmakoIF_csharp;

namespace SaveWithOutputIntentCS;

class SaveWithOutputIntent
{
    static void Main(string[] args)
    {
        // The ICC profiles
        var profiles = new List<Tuple<string, string>>();
        profiles.Add(Tuple.Create("Probev2_ICCv4.icc", @"https://www.color.org/probeprofile.xalter"));
        profiles.Add(Tuple.Create("GRACoL2006_Coated1v2.icc", "https://idealliance.org"));
        profiles.Add(Tuple.Create("JapanColor2011Coated.icc", "https://www.xrite.com"));

        try
        {
            var mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);

            // Get a page ready to accept some DOM
            using var assembly = IDocumentAssembly.create(mako);
            using var document = IDocument.create(mako);
            assembly.appendDocument(document);
            using var page = IPage.create(mako);
            document.appendPage(page);
            var fixedPage = IDOMFixedPage.create(mako, M2X(250), M2X(210));
            page.setContent(fixedPage);

            // Get a font
            var font = IDOMFontOpenType.fromRCObject(mako.findFont("Arial", out var fontIndex).toRCObject());

            // Create a colour
            var darkBlue = IDOMColor.createSolidRgb(mako, 0.0f, 0.0f, 0.5f);

            // Create a layout
            var layout = ILayout.create(mako);

            // Add frame to hold content
            layout.addFrame(ILayoutFrame.create(new FRect(M2X(12), M2X(12), M2X(273), M2X(186))));

            // Create template paragraphs
            var header = ILayoutParagraph.create(ILayoutParagraph.eHorizontalAlignment.eHALeft, P2X(6));
            var body = ILayoutParagraph.create(ILayoutParagraph.eHorizontalAlignment.eHALeft, P2X(5));

            // A vector to point to each paragraph to be added to the frame(s)
            CEDLVectILayoutParagraph paragraphs = new CEDLVectILayoutParagraph();
            uint paraIndex = 0;

            // Create paragraphs and text runs 
            var text = "This PDF has an Output Intent referencing this ICC profile:";
            var run = ILayoutTextRun.create(text, font, fontIndex, P2X(12), darkBlue);
            paragraphs.append(header.clone());
            paragraphs[paraIndex].addRun(ILayoutRun.fromRCObject(run.toRCObject()));
            text = $"  ● {profiles[0].Item1} (from {profiles[0].Item2})";
            run = ILayoutTextRun.create(text, font, fontIndex, P2X(10));
            paragraphs.append(body.clone());
            paragraphs[++paraIndex].addRun(ILayoutRun.fromRCObject(run.toRCObject()));

            // Picture
            var parrot = GetImage(ref mako, @"..\..\..\..\TestFiles\Parrot.png");
            paragraphs.append(ILayoutParagraph.create());
            paragraphs[++paraIndex].addRun(ILayoutImageRun.create(mako, parrot, 0, M2X(169)));

            // Add content to the page
            fixedPage.appendChild(layout.layout(paragraphs));

            // Load profile from local storage
            var iccProfile = IDOMICCProfile.create(mako, IInputStream.createFromFile(mako, Path.Combine(@"..\..\..\..\TestFiles\", profiles[0].Item1)));

            // Output intent metadata
            var subtype = "GTS_PDFX";
            var registryName = "http://www.color.org";
            var outputCondition = profiles[0].Item1;
            var outputConditionIdentifier = profiles[0].Item1;
            var info = "Output Intent test";

            // Create a PDF output set to write PDF/X
            using var output = IPDFOutput.create(mako);
            output.setPreset("PDF/X-4");

            // Create an intent and add to PDF
            var outputIntent = IOutputIntent.create(mako, subtype, outputCondition, outputConditionIdentifier, registryName, info, iccProfile);
            output.setOutputIntent(outputIntent);

            // Write PDF
            output.writeAssembly(assembly, "OutputIntentExample.pdf");

            // Check that the PDF has an output intent
            var testDocument = IPDFInput.create(mako).open("OutputIntentExample.pdf").getDocument();
            if (testDocument.getOutputIntents().size() != 1)
                throw new ApplicationException("Output intent count not equal to 1");

            // Demonstration of adding more than one output intent. PDF/X allows it, may suit some custom workflows
            CEDLVectIOutputIntent outputIntents = new CEDLVectIOutputIntent();
            outputIntents.append(outputIntent);

            // We add two alternates
            subtype = "GGS_DUMMY";
            registryName = "https://idealliance.org";
            outputCondition = profiles[1].Item1;
            outputConditionIdentifier = profiles[1].Item1;
            info = "The GRACoL 2006 Coated v2 from idealliance.org";

            // Create an intent and add to list
            outputIntent = IOutputIntent.create(mako, subtype, outputCondition, outputConditionIdentifier, registryName, info, iccProfile);
            outputIntents.append(outputIntent);

            subtype = "GGS_DUMMY";
            registryName = "https://www.xrite.com";
            outputCondition = profiles[2].Item1;
            outputConditionIdentifier = profiles[2].Item1;
            info = "The Japan Color 2011 profile from xrite.com";

            // Create an intent and add to list
            outputIntent = IOutputIntent.create(mako, subtype, outputCondition, outputConditionIdentifier, registryName, info, iccProfile);
            outputIntents.append(outputIntent);

            // Set multiple intents
            output.setOutputIntents(outputIntents);

            // Write PDF
            output.writeAssembly(assembly, "OutputIntentExampleThreeIntents.pdf");

            // Check that the PDF has three output intents
            testDocument = IPDFInput.create(mako).open("OutputIntentExampleThreeIntents.pdf").getDocument();
            if (testDocument.getOutputIntents().size() != 3)
                throw new ApplicationException("Output intent count not equal to 3");

            // Done
        }
        catch (MakoException e)
        {
            var errorFormatString = getEDLErrorString(e.m_errorCode);
            Console.WriteLine("Exception thrown: " + e.m_msg);
        }
        catch (Exception e)
        {
            Console.WriteLine($"Exception thrown: {e}");
        }
    }

    // Points (1/72") to XPS units (1/96")
    public static double P2X(double value)
    {
        return value / 72.0 * 96.0;
    }

    // Millimetres to XPS units (1/96")
    public static double M2X(double value)
    {
        return value / 25.4 * 96.0;
    }

    // Get image 
    private static IDOMImage GetImage(ref IJawsMako mako, string imageFile)
    {
        var fileInfo = new FileInfo(imageFile);
        if (!fileInfo.Exists)
        {
            throw new FileNotFoundException($"Image file {imageFile} not found.");
        }

        IDOMImage? image = null;

        if (fileInfo.Extension == ".jpg")
            image = IDOMJPEGImage.create(mako, IInputStream.createFromFile(mako, imageFile));

        if (fileInfo.Extension == ".png")
            image = IDOMPNGImage.create(mako, IInputStream.createFromFile(mako, imageFile));

        if (fileInfo.Extension == ".tif")
            image = IDOMTIFFImage.create(mako, IInputStream.createFromFile(mako, imageFile));

        if (image == null)
        {
            throw new Exception($"Image file {imageFile} could not be loaded.");
        }

        return image;
    }
}