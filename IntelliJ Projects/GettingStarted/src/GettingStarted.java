/* -----------------------------------------------------------------------
 * <copyright file="Main.java" company="Hybrid Software Helix Ltd">
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

public class GettingStarted {
    public static void main(String[] args) {

        var jawsMako = IJawsMako.create();
        var factory = jawsMako.getFactory();
        IJawsMako.enableAllFeatures(jawsMako);
        var assembly = IDocumentAssembly.create(jawsMako);
        var document = IDocument.create(jawsMako);
        assembly.appendDocument(document);
        var page = IPage.create(jawsMako);
        document.appendPage(page);

        // Create a fixed page A5 landscape.
        double pageWidth = 793.7;
        double pageHeight = 561.1;
        var fixedPage = IDOMFixedPage.create(factory, pageWidth, pageHeight);

        // Now create some DOM for the page contents.

        // Find a font
        long[] fontIndex = {0};
        IDOMFont font = jawsMako.findFont("Arial Bold", fontIndex);

        // Create a blue brush
        var colorSpace = IDOMColorSpacesRGB.create (factory);
        var colorValues = new StdVectFloat();
        colorValues.add(0.0f);
        colorValues.add(0.0f);
        colorValues.add(1.0f);

        var blueColor = IDOMColor.createFromVect(factory, colorSpace, 1.0f, colorValues);
        var solidBrush = IDOMSolidColorBrush.create(factory, blueColor);

        // Create glyphs
        String message = "Mako Java API";
        var glyphs = IDOMGlyphs.create(factory, message, 72, new FPoint(10, pageHeight / 2), solidBrush, font, fontIndex[0]);

        // And add to the page
        fixedPage.appendChild(glyphs.toIDOMNode());


        // Assign content to a page
        page.setContent(fixedPage);

        // Now we can write this to a PDF
        IPDFOutput.create(jawsMako).writeAssembly(assembly, "TestJava.pdf");

        System.out.println("File created.");
    }
}