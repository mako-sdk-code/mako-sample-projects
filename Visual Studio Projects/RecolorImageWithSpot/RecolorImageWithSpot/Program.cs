/* -----------------------------------------------------------------------
 * <copyright file="MakoRecolorImageWithSpot.cs" company="Hybrid Software Helix Ltd">
 *   Copyright (c) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * 
 * <summary>
 *   Recolor Image With Spot Example
 *   Demonstrates how to paint an image with a spot color
 *   
 *   This example is provided on an "as is" basis and without warranty of any kind.
 *   Hybrid Software Helix Ltd. does not warrant or make any representations regarding the use or
 *   results of use of this example.
 *   </summary>
 * -----------------------------------------------------------------------
 */

using JawsMako;

namespace MakoRecolorImageWithSpot;

internal class Program
{
    static int Main()
    {
        try
        {
            var testFilepath = @"..\..\..\..\TestFiles\";

            var mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);

            // Create a spot color and corresponding solid color brush
            var rubineRed = MakeSeparationColor(mako, "Pantone Rubine Red C", [43.92, 79.0, 13.0]);
            var rubineRedBrush = IDOMSolidColorBrush.create(mako, rubineRed);

            // Load an image from disk
            var image = IDOMPNGImage.create(mako, IInputStream.createFromFile(mako, testFilepath + "Cheshire Cat.png"));

            // Get image details
            var imageFrame = image.getImageFrame(mako);
            var imageColorSpace = imageFrame.getColorSpace();
            var imageWidth = imageFrame.getWidth();
            var imageHeight = imageFrame.getHeight();
            var imageXres = imageFrame.getXResolution();
            var imageYres = imageFrame.getYResolution();
            var imageBounds = new FRect(0.0, 0.0, imageWidth / imageXres * 96.0, imageHeight / imageYres * 96.0);

            // Color convert to grayscale and invert
            var imageFilters = new CEDLVectIDOMImageFilter();
            if (imageColorSpace.getNumComponents() != 1)
                imageFilters.append(
                    IDOMImageColorConverterFilter.create(mako, IDOMColorSpaceDeviceGray.create(mako),
                        eRenderingIntent.ePerceptual, eBlackPointCompensation.eBPCDefault));
            imageFilters.append(IDOMImageInverterFilter.create(mako));

            // Apply filter(s) to create a mask
            var mask = IDOMFilteredImage.create(mako, image, imageFilters);

            // Create a masked brush
            var maskedBrush = IDOMMaskedBrush.create(mako, mask, rubineRedBrush, imageBounds, imageBounds);

            // Create a path of suitable size and paint with masked brush
            var path = IDOMPathNode.createFilled(mako, IDOMPathGeometry.create(mako, imageBounds), maskedBrush);

            // Create a document assembly, document, page and fixed page
            var assembly = IDocumentAssembly.create(mako);
            var document = IDocument.create(mako);
            assembly.appendDocument(document);
            var page = IPage.create(mako);
            document.appendPage(page);
            var fixedPage = IDOMFixedPage.create(mako);
            page.setContent(fixedPage);

            // Scale the image to fit the page
            path.setRenderTransform(new FMatrix(imageBounds, page.getCropBox()));

            // Add to page
            fixedPage.appendChild(path);

            // Save to PDF
            IPDFOutput.create(mako).writeAssembly(assembly, IOutputStream.createToFile(mako, "Cheshire Cat.pdf"));
        }
        catch (MakoException e)
        {
            Console.WriteLine($"Exception thrown: {e.m_errorCode}: {e.m_msg}");
        }
        catch (Exception e)
        {
            Console.WriteLine($"Exception thrown: {e}");
        }

        return 0;
    }

    // Define an L*a*b spot color
    static IDOMColor MakeSeparationColor(IJawsMako mako, string name, double[] labRepresentation)
    {
        // Create a vector of colorants with one entry
        var colorantInfo = new IDOMColorSpaceDeviceN.CColorantInfo(name, new CEDLVectDouble(labRepresentation));

        var colorants = new CEDLVectColorantInfo();
        colorants.append(colorantInfo);

        var alternate = IDOMColorSpaceLAB.create(mako, 0.9642f, 1.0000f, 0.8249f, 0.0f, 0.0f, 0.0f, -128f, 127f,
            -128f, 127f);
        IDOMColorSpaceDeviceN colorSpace =
            IDOMColorSpaceDeviceN.create(mako.getFactory(), colorants, alternate.toIDOMColorSpace());

        // Return the new spot color
        return IDOMColor.createFromArray(mako.getFactory(), colorSpace, 1f, [1f]);
    }

    // Define a CMYK spot color
    // ReSharper disable once UnusedMember.Local
    static IDOMColor MakeCmykSeparationColor(IJawsMako mako, string name, double[] cmykRepresentation)
    {
        // Create a vector of colorants with one entry
        var colorantInfo = new IDOMColorSpaceDeviceN.CColorantInfo(name, new CEDLVectDouble(cmykRepresentation));

        var colorants = new CEDLVectColorantInfo();
        colorants.append(colorantInfo);

        var alternate = IDOMColorSpaceDeviceCMYK.create(mako);
        IDOMColorSpaceDeviceN colorSpace =
            IDOMColorSpaceDeviceN.create(mako.getFactory(), colorants, alternate.toIDOMColorSpace());

        // Return the new spot color
        return IDOMColor.createFromArray(mako.getFactory(), colorSpace, 1f, [1f]);
    }
}