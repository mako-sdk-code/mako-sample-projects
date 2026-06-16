/* --------------------------------------------------------------------------------
 *  <copyright file="Program.cs" company="Hybrid Software Helix Ltd">
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

namespace AddImageToPdfCS;

internal class AddImageToPdfCS
{
    private const string TestFilesPath = @"..\..\..\..\..\..\TestFiles\";

    static int Main()
    {
        try
        {
            using var mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);
            using var assembly = IDocumentAssembly.create(mako);
            using var document = IDocument.create(mako);
            assembly.appendDocument(document);
            using var page = IPage.create(mako);
            document.appendPage(page);
            using var fixedPage = IDOMFixedPage.create(mako);
            page.setContent(fixedPage);

            using var image = IDOMPNGImage.create(mako, IInputStream.createFromFile(mako, TestFilesPath + "Cheshire Cat.png"));

            // Get image attributes
            using var imageFrame = image.getImageFrame(mako);
            var width = imageFrame.getWidth();
            var height = imageFrame.getHeight();

            using var imageNode = IDOMPathNode.createImage(mako, image, new FRect(0.0, 0.0, width, height));
            fixedPage.appendChild(imageNode);

            IPDFOutput.create(mako).writeAssembly(assembly, IOutputStream.createToFile(mako, "test.pdf"));
        }
        catch (MakoException e)
        {
            Console.Error.WriteLine($"Exception thrown: {e.m_errorCode}: {e.m_msg}");
            return 1;
        }
        catch (Exception e)
        {
            Console.Error.WriteLine($"Exception thrown: {e}");
            return 1;
        }

        return 0;
    }
}