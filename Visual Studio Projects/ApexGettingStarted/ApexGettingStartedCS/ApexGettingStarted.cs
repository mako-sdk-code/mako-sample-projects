/* -----------------------------------------------------------------------
 * <copyright file="ApexGettingStarted.cs" company="Hybrid Software Helix Ltd">
 *  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

using JawsMako;

namespace ApexRenderExampleCS;

internal class ApexGettingStarted
{
    static int Main(string[] args)
    {
        try
        {
            const string testFilePath = "..\\..\\..\\..\\TestFiles\\";
            const string resultFilePath = "..\\..\\..\\..\\TestFiles\\";

            var mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);

            // Open the document assembly, the first document, the first page and get the content thereof
            var input = IInput.create(mako, eFileFormat.eFFPDF);
            var assembly = input.open(testFilePath + "Cheshire Cat.pdf");
            var document = assembly.getDocument();
            var page = document.getPage();
            var cropBox = page.getCropBox();
            var content = page.getContent();

            // Let's render with Apex
            var renderer = IApexRenderer.create(mako);

            // First, we need a renderspec; in this case, to render to an IDOMImage
            var imageRenderSpec = new CImageRenderSpec();

            // We want a 300dpi result, so calculate the size in pixels
            imageRenderSpec.width = (uint)(page.getWidth() / 96.0 * 300.0);
            imageRenderSpec.height = (uint)(page.getHeight() / 96.0 * 300.0);

            // Specify the area to be rendered
            imageRenderSpec.sourceRect = cropBox;

            // And the color space
            imageRenderSpec.processSpace = IDOMColorSpaceDeviceRGB.create(mako);

            // Now render
            renderer.render(content, imageRenderSpec);

            // Fetch the result and encode in an image
            var image = imageRenderSpec.result;
            IDOMPNGImage.encode(mako, image, IOutputStream.createToFile(mako, resultFilePath + "Cheshire Cat.png"));
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
}