/* -----------------------------------------------------------------------
 * <copyright file="SaveWithOutputIntent.java" company="Hybrid Software Helix Ltd">
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
import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class SaveWithOutputIntent {

    public static void main(String[] args) {
        try {
            // Define ICC profiles and their URLs
            List<String[]> profiles = new ArrayList<>();
            profiles.add(new String[]{"Probev2_ICCv4.icc", "https://www.color.org/probeprofile.xalter"});
            profiles.add(new String[]{"GRACoL2006_Coated1v2.icc", "https://idealliance.org"});
            profiles.add(new String[]{"JapanColor2011Coated.icc", "https://www.xrite.com"});

            IJawsMako mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);

            IEDLClassFactory factory = mako.getFactory();

            // Create a document and page
            IDocumentAssembly assembly = IDocumentAssembly.create(mako);
            IDocument document = IDocument.create(mako);
            assembly.appendDocument(document);

            IPage page = IPage.create(mako);
            document.appendPage(page);

            IDOMFixedPage fixedPage = IDOMFixedPage.create(factory, M2X(250), M2X(210));
            page.setContent(fixedPage);

            // Get font
            long[] fontIndex = new long[1];
            var fontHandle = mako.findFont("Arial", fontIndex);
            IDOMFontOpenType font = IDOMFontOpenType.fromRCObject(fontHandle.toRCObject());

            // Create color and layout
            IDOMColor darkBlue = IDOMColor.createSolidRgb(factory, 0.0f, 0.0f, 0.5f);
            ILayout layout = ILayout.create(mako);
            layout.addFrame(ILayoutFrame.create(new FRect(M2X(12), M2X(12), M2X(273), M2X(186))));

            ILayoutParagraph header = ILayoutParagraph.create(ILayoutParagraph.eHorizontalAlignment.eHALeft, P2X(6));
            ILayoutParagraph body = ILayoutParagraph.create(ILayoutParagraph.eHorizontalAlignment.eHALeft, P2X(5));

            CEDLVectILayoutParagraph paragraphs = new CEDLVectILayoutParagraph();
            int paraIndex = 0;

            // Header text
            String text = "This PDF has an Output Intent referencing this ICC profile:";
            ILayoutTextRun run = ILayoutTextRun.create(text, font, fontIndex[0], P2X(12), darkBlue);
            paragraphs.append(header.clone());
            paragraphs.getitem(paraIndex).addRun(ILayoutRun.fromRCObject(run.toRCObject()));

            // Body text
            text = "  ● " + profiles.getFirst()[0] + " (from " + profiles.getFirst()[1] + ")";
            run = ILayoutTextRun.create(text, font, fontIndex[0], P2X(10));
            paragraphs.append(body.clone());
            paraIndex++;
            paragraphs.getitem(paraIndex).addRun(ILayoutRun.fromRCObject(run.toRCObject()));

            // Insert image
            IDOMImage parrot = GetImage(mako, "TestFiles/Parrot.png");
            paragraphs.append(ILayoutParagraph.create());
            paraIndex++;
            paragraphs.getitem(paraIndex).addRun(ILayoutImageRun.create(mako, parrot, 0, M2X(169)));

            // Add content to page
            fixedPage.appendChild(layout.layout(paragraphs));

            // Load ICC profile from file
            IDOMICCProfile iccProfile = IDOMICCProfile.create(factory,
                    IInputStream.createFromFile(factory, "TestFiles/" + profiles.getFirst()[0]));

            // Create PDF output with OutputIntent
            IPDFOutput output = IPDFOutput.create(mako);
            output.setPreset("PDF/X-4");

            String subtype = "GTS_PDFX";
            String registryName = "http://www.color.org";
            String outputCondition = profiles.getFirst()[0];
            String outputConditionIdentifier = profiles.getFirst()[0];
            String info = "Output Intent test";

            IOutputIntent outputIntent = IOutputIntent.create(mako, subtype,
                    outputCondition, outputConditionIdentifier, registryName, info, iccProfile);
            output.setOutputIntent(outputIntent);

            // Write PDF
            output.writeAssembly(assembly, "OutputIntentExample.pdf");

            // Verify single intent
            IDocument testDocument = IPDFInput.create(mako)
                    .open("OutputIntentExample.pdf")
                    .getDocument();
            if (testDocument.getOutputIntents().size() != 1)
                throw new Exception("Output intent count not equal to 1");

            // Add multiple output intents
            CEDLVectIOutputIntent outputIntents = new CEDLVectIOutputIntent();
            outputIntents.append(outputIntent);

            // Add GRACoL
            subtype = "GGS_DUMMY";
            registryName = "https://idealliance.org";
            outputCondition = profiles.get(1)[0];
            outputConditionIdentifier = profiles.get(1)[0];
            info = "The GRACoL 2006 Coated v2 from idealliance.org";
            outputIntent = IOutputIntent.create(mako, subtype, outputCondition,
                    outputConditionIdentifier, registryName, info, iccProfile);
            outputIntents.append(outputIntent);

            // Add JapanColor
            subtype = "GGS_DUMMY";
            registryName = "https://www.xrite.com";
            outputCondition = profiles.get(2)[0];
            outputConditionIdentifier = profiles.get(2)[0];
            info = "The Japan Color 2011 profile from xrite.com";
            outputIntent = IOutputIntent.create(mako, subtype, outputCondition,
                    outputConditionIdentifier, registryName, info, iccProfile);
            outputIntents.append(outputIntent);

            output.setOutputIntents(outputIntents);
            output.writeAssembly(assembly, "OutputIntentExampleThreeIntents.pdf");

            // Verify three intents
            testDocument = IPDFInput.create(mako)
                    .open("OutputIntentExampleThreeIntents.pdf")
                    .getDocument();
            if (testDocument.getOutputIntents().size() != 3)
                throw new Exception("Output intent count not equal to 3");

            System.out.println("Output intents successfully created and verified.");

        } catch (Exception e) {
            System.out.println("Exception thrown: " + e);
        }
    }

    // Points (1/72") to XPS units (1/96")
    public static double P2X(double value) {
        return value / 72.0 * 96.0;
    }

    // Millimetres to XPS units (1/96")
    public static double M2X(double value) {
        return value / 25.4 * 96.0;
    }

    // Load image file
    private static IDOMImage GetImage(IJawsMako mako, String imageFile) throws Exception {
        File file = new File(imageFile);
        if (!file.exists()) {
            throw new Exception("Image file " + imageFile + " not found.");
        }

        String ext = imageFile.substring(imageFile.lastIndexOf('.')).toLowerCase();
        IDOMImage image = switch (ext) {
            case ".jpg" ->
                    IDOMJPEGImage.create(mako.getFactory(), IInputStream.createFromFile(mako.getFactory(), imageFile));
            case ".png" ->
                    IDOMPNGImage.create(mako.getFactory(), IInputStream.createFromFile(mako.getFactory(), imageFile));
            case ".tif" ->
                    IDOMTIFFImage.create(mako.getFactory(), IInputStream.createFromFile(mako.getFactory(), imageFile));
            default -> null;
        };

        if (image == null)
            throw new Exception("Image file " + imageFile + " could not be loaded.");

        return image;
    }
}
