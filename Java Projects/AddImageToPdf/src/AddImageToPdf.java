/* --------------------------------------------------------------------------------
 *  <copyright file="Main.java" company="Hybrid Software Helix Ltd">
 *    Copyright (c) 2025 Hybrid Software Helix Ltd. All rights reserved.
 *  </copyright>
 *  <summary>
 *    This example is provided on an "as is" basis and without warranty of any kind.
 *    Hybrid Software Helix Ltd. does not warrant or make any representations
 *    regarding the use or results of use of this example.
 *  </summary>
 * ---------------------------------------------------------------------------------
 */

import com.globalgraphics.JawsMako.jawsmakoIF.*;

public class AddImageToPdf {
    public static void main(String[] args) {

        var testFilepath = "TestFiles\\";

        var jawsMako = IJawsMako.create();
        var factory = jawsMako.getFactory();
        IJawsMako.enableAllFeatures(jawsMako);
        var assembly = IDocumentAssembly.create(jawsMako);
        var document = IDocument.create(jawsMako);
        assembly.appendDocument(document);
        var page = IPage.create(jawsMako);
        document.appendPage(page);
        var fixedPage = IDOMFixedPage.create(factory);
        page.setContent(fixedPage);

        // Load image from file
        var image = IDOMPNGImage.create(factory, IInputStream.createFromFile(factory,testFilepath + "makologo.png"));

        // Get image attributes
        var imageFrame = image.getImageFrame(factory);
        var width = imageFrame.getWidth();
        var height = imageFrame.getHeight();

        // Add image to fixedPage
        var imageNode = IDOMPathNode.createImage(factory, image, new FRect(0.0, 0.0, width, height));
        fixedPage.appendChild(imageNode);

        // Now we can write this to a PDF
        IPDFOutput.create(jawsMako).writeAssembly(assembly, "TestJava.pdf");

        System.out.println("File created.");
    }
}