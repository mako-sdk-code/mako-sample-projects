/* -----------------------------------------------------------------------
 * <copyright file="MakoRecolorImageWithSpot.java" company="Hybrid Software Helix Ltd">
 *  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  Recolor Image With Spot Example
 *  Demonstrates how to paint an image with a spot color
 *
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

import com.globalgraphics.JawsMako.jawsmakoIF.*;

public class MakoRecolorImageWithSpot {
    public static void main(String[] args) {
        try {
            String testFilepath = "TestFiles/";

            IJawsMako mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);

            // Create a spot color and corresponding solid color brush
            IDOMColor rubineRed = makeSeparationColor(mako, "Pantone Rubine Red C", new double[]{43.92, 79.0, 13.0});
            IDOMSolidColorBrush rubineRedBrush = IDOMSolidColorBrush.create(mako.getFactory(), rubineRed);

            // Load an image from disk
            IDOMPNGImage image = IDOMPNGImage.create(mako.getFactory(), IInputStream.createFromFile(mako.getFactory(), testFilepath + "Cheshire Cat.png"));

            // Get image details
            IImageFrame imageFrame = image.getImageFrame(mako.getFactory());
            IDOMColorSpace imageColorSpace = imageFrame.getColorSpace();
            double imageWidth = imageFrame.getWidth();
            double imageHeight = imageFrame.getHeight();
            double imageXres = imageFrame.getXResolution();
            double imageYres = imageFrame.getYResolution();
            FRect imageBounds = new FRect(0.0, 0.0, imageWidth / imageXres * 96.0, imageHeight / imageYres * 96.0);

            // Color convert to grayscale and invert
            CEDLVectIDOMImageFilter imageFilters = new CEDLVectIDOMImageFilter();
            if (imageColorSpace.getNumComponents() != 1) {
                imageFilters.append(
                        IDOMImageColorConverterFilter.create(mako.getFactory(), IDOMColorSpaceDeviceGray.create(mako.getFactory()),
                                eRenderingIntent.ePerceptual, eBlackPointCompensation.eBPCDefault));
            }
            imageFilters.append(IDOMImageInverterFilter.create(mako.getFactory()));

            // Apply filters to create a mask
            IDOMFilteredImage mask = IDOMFilteredImage.create(mako.getFactory(), image, imageFilters);

            // Create a masked brush
            IDOMMaskedBrush maskedBrush = IDOMMaskedBrush.create(mako.getFactory(), mask, rubineRedBrush, imageBounds, imageBounds);

            // Create a path of suitable size and paint with masked brush
            IDOMPathNode path = IDOMPathNode.createFilled(mako.getFactory(), IDOMPathGeometry.create(mako.getFactory(), imageBounds), maskedBrush);

            // Create document assembly, document, page, and fixed page
            IDocumentAssembly assembly = IDocumentAssembly.create(mako);
            IDocument document = IDocument.create(mako);
            assembly.appendDocument(document);
            IPage page = IPage.create(mako);
            document.appendPage(page);
            IDOMFixedPage fixedPage = IDOMFixedPage.create(mako.getFactory());
            page.setContent(fixedPage);

            // Scale the image to fit the page
            path.setRenderTransform(new FMatrix(imageBounds, page.getCropBox()));

            // Add to page
            fixedPage.appendChild(path);

            // Save to PDF
            IPDFOutput.create(mako).writeAssembly(assembly, IOutputStream.createToFile(mako.getFactory(), "Cheshire Cat.pdf"));
        }
        catch (Exception e) {
            System.out.println("Exception thrown: " + e);
        }
    }

    // Define an L*a*b spot color
    static IDOMColor makeSeparationColor(IJawsMako mako, String name, double[] labRepresentation) {
        // Create a vector of colorants with one entry
        IDOMColorSpaceDeviceN.CColorantInfo colorantInfo = new IDOMColorSpaceDeviceN.CColorantInfo(name, new CEDLVectDouble(labRepresentation));

        CEDLVectColorantInfo colorants = new CEDLVectColorantInfo();
        colorants.append(colorantInfo);

        IDOMColorSpaceLAB alternate = IDOMColorSpaceLAB.create(mako.getFactory(), 0.9642f, 1.0000f, 0.8249f, 0.0f, 0.0f, 0.0f, -128f, 127f, -128f, 127f);
        IDOMColorSpaceDeviceN colorSpace =
                IDOMColorSpaceDeviceN.create(mako.getFactory(), colorants, alternate.toIDOMColorSpace());

        // Return the new spot color
        return IDOMColor.createFromArray(mako.getFactory(), colorSpace, 1f, new float[]{1f});
    }

    // Define a CMYK spot color
    static IDOMColor makeCmykSeparationColor(IJawsMako mako, String name, double[] cmykRepresentation) {
        // Create a vector of colorants with one entry
        IDOMColorSpaceDeviceN.CColorantInfo colorantInfo = new IDOMColorSpaceDeviceN.CColorantInfo(name, new CEDLVectDouble(cmykRepresentation));

        CEDLVectColorantInfo colorants = new CEDLVectColorantInfo();
        colorants.append(colorantInfo);

        IDOMColorSpaceDeviceCMYK alternate = IDOMColorSpaceDeviceCMYK.create(mako.getFactory());
        IDOMColorSpaceDeviceN colorSpace =
                IDOMColorSpaceDeviceN.create(mako.getFactory(), colorants, alternate.toIDOMColorSpace());

        // Return the new spot color
        return IDOMColor.createFromArray(mako.getFactory(), colorSpace, 1f, new float[]{1f});
    }
}
