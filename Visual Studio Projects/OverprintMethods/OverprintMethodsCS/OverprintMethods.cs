/* --------------------------------------------------------------------------------
 *  <copyright file="OverprintMethods.cs" company="Global Graphics Software Ltd">
 *    Copyright (c) 2025 Global Graphics Software Ltd. All rights reserved.
 *  </copyright>
 *  <summary>
 *    This example is provided on an "as is" basis and without warranty of any kind.
 *    Global Graphics Software Ltd. does not warrant or make any representations
 *    regarding the use or results of use of this example.
 *  </summary>
 * ---------------------------------------------------------------------------------
 */

using JawsMako;

namespace EncodePNGinMemory;

internal class OverprintMethods
{

    static int Main(string[] args)
    {
        var testFilepath = @"..\..\..\..\TestFiles\";

        try
        {
            var mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);
            using var assembly = IPDFInput.create(mako).open(testFilepath + "CMYK_Circles 1.pdf");
            using var document = assembly.getDocument();

            for (uint pageIndex = 0; pageIndex < document.getNumPages(); ++pageIndex)
            {
                IPage page = document.getPage(pageIndex);
                IDOMFixedPage fixedPage = page.edit();

                var pathNodes = fixedPage.findChildrenOfType(eDOMNodeType.eDOMPathNode, true);

                foreach (var node in pathNodes.toVector())
                {
                    IDOMPathNode path = IDOMPathNode.fromRCObject(node);
                    if (path != null)
                    {
                        path.setFillOverprints(true);
                        path.setOverprintMode(true);
                    }
                }
            }

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
