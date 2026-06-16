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

namespace OverprintMethodsCS;

internal class OverprintMethods
{
    private const string TestFilesPath = @"..\..\..\..\..\..\TestFiles\";
    
    static int Main(string[] args)
    {
        try
        {
            var mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);
            using var assembly = IPDFInput.create(mako).open(TestFilesPath + "CMYK_Circles 1.pdf");
            using var document = assembly.getDocument();

            for (uint pageIndex = 0; pageIndex < document.getNumPages(); ++pageIndex)
            {
                using var page = document.getPage(pageIndex);
                using var fixedPage = page.edit();

                using var pathNodes = fixedPage.findChildrenOfType(eDOMNodeType.eDOMPathNode, true);

                foreach (var node in pathNodes.toVector().Select(IDOMPathNode.fromRCObject))
                {
                    if (node == null) continue;
                    node.setFillOverprints(true);
                    node.setOverprintMode(true);
                }
            }

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