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
using static JawsMako.jawsmakoIF_csharp;

namespace EncodePNGinMemory
{
    internal class AddImageToPdfCS
    {
        static int Main(string[] args)
        {
            try
            {
                var testFilepath = @"..\..\..\..\TestFiles\";

                var mako = IJawsMako.create();
                IJawsMako.enableAllFeatures(mako);
                var assembly = IDocumentAssembly.create(mako);
                var document = IDocument.create(mako);
                assembly.appendDocument(document);
                var page = IPage.create(mako);
                document.appendPage(page);
                var fixedPage = IDOMFixedPage.create(mako);
                page.setContent(fixedPage);

                var image = IDOMPNGImage.create(mako, IInputStream.createFromFile(mako, testFilepath + "Cheshire Cat.png"));

                // Get image attributes
                var imageFrame = image.getImageFrame(mako);
                var width = imageFrame.getWidth();
                var height = imageFrame.getHeight();

                var imageNode = IDOMPathNode.createImage(mako, image, new FRect(0.0, 0.0, width, height));
                fixedPage.appendChild(imageNode);

                IPDFOutput.create(mako).writeAssembly(assembly, IOutputStream.createToFile(mako, "test.pdf"));
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
}