/* -----------------------------------------------------------------------
 * <copyright file="ApexGettingStarted.java" company="Hybrid Software Helix Ltd">
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
import com.globalgraphics.JawsMako.jawsmakoIF.jawsmakoIF.*;

public class ApexGettingStarted {
    public static void main(String[] args) {
        try {
            // Create our JawsMako instance.
            IJawsMako jawsMako = IJawsMako.create();
            IJawsMako.enableAllFeatures(jawsMako);

            String testFilePath = "TestFiles\\";
            String resultFilePath = "TestFiles\\";

            // Open the document assembly, the first document, the first page and get the content thereof
            IInput input = IInput.create(jawsMako, eFileFormat.eFFPDF);
            IDocumentAssembly assembly = input.open(testFilePath + "Cheshire Cat.pdf");
            IDocument document = assembly.getDocument();
            IPage page = document.getPage(0);
            FRect cropBox = page.getCropBox();
            IDOMFixedPage content = page.getContent();

            // Let's render with Apex
            IApexRenderer renderer = IApexRenderer.create(jawsMako);

            // First we need a renderspec, in this case to render to an IDOMImage
            CImageRenderSpec imageRenderSpec = new CImageRenderSpec();

            // We want a 300dpi result, so calculate the size in pixels
            imageRenderSpec.setWidth((long)(page.getWidth() / 96.0 * 300.0));
            imageRenderSpec.setHeight((long)(page.getHeight() / 96.0 * 300.0));

            // Specify the area to be rendered
            imageRenderSpec.setSourceRect(cropBox);

            // And the color space
            imageRenderSpec.setProcessSpace(IDOMColorSpaceDeviceRGB.create(jawsMako.getFactory()));

            // Now render
            renderer.render(content, imageRenderSpec);

            // Fetch the result and encode in an image
            IDOMImage image = imageRenderSpec.getResult();
            IDOMPNGImage.encode(jawsMako, image,
                    IOutputStream.createToFile(jawsMako.getFactory(), resultFilePath + "Cheshire Cat.png"));

            System.out.println("Render complete. Output written to " + resultFilePath + "Cheshire Cat.png");
        }
        catch (Exception e) {
            System.err.println("Exception thrown: " + e.getMessage());
        }
    }
}
