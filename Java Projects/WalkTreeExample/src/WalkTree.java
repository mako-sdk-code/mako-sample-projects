/* --------------------------------------------------------------------------------
 *  <copyright file="WalkTree.java" company="Hybrid Software Helix Ltd">
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
import com.globalgraphics.JawsMako.jawsmakoIF.jawsmakoIF.*;

public class WalkTree
{
    public static void main(String[] args)
    {
        try
        {
            String testFilepath = "TestFiles/";

            // Instantiate Mako
            IJawsMako mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);

            // Input
            IPDFInput pdfInput = IPDFInput.create(mako);
            IDocumentAssembly assembly = pdfInput.open(testFilepath + "Cheshire Cat.pdf");

            IDOMFixedPage inFixedPage = assembly.getDocument().getPage(0).getContent();

            // Define the callback and walk the tree
            WalkTreeCallback callback = new NodeTreeCallback();
            inFixedPage.walkTree(callback.getCallbackFunc(), callback.getPriv(), false, true);
        }
        catch (Exception e)
        {
            System.out.printf("Exception thrown: %s%n", e.toString());
        }
    }

    private static class NodeTreeCallback extends WalkTreeCallback
    {
        @Override
        public boolean visitNode(IDOMNode node)
        {
            dumpNodeInfo(node);
            return true;
        }
    }

    private static void dumpNodeInfo(IDOMNode node)
    {
        IDOMNode p = node;
        while (p.getParentNode() != null)
        {
            System.out.print("  ");
            p = p.getParentNode();
        }

        switch (node.getNodeType())
        {
            case eDOMFixedPageNode: System.out.print("eDOMFixedPageNode"); break;
            case eDOMGroupNode: System.out.print("eDOMGroupNode"); break;
            case eDOMCharPathGroupNode: System.out.print("eDOMCharPathGroupNode"); break;
            case eDOMTransparencyGroupNode: System.out.print("eDOMTransparencyGroupNode"); break;
            case eDOMGlyphsNode: System.out.print("eDOMGlyphsNode"); break;
            case eDOMPathNode: System.out.print("eDOMPathNode"); break;
            case eDOMFormNode: System.out.print("eDOMFormNode"); break;
            case eDOMFormInstanceNode: System.out.print("eDOMFormInstanceNode"); break;
            case eDOMContentRootNode: System.out.print("eDOMContentRootNode"); break;
            case eDOMDocumentSequenceNode: System.out.print("eDOMDocumentSequenceNode"); break;
            case eDOMDocumentNode: System.out.print("eDOMDocumentNode"); break;
            case eDOMFixedDocumentNode: System.out.print("eDOMFixedDocumentNode"); break;
            case eDOMPageNode: System.out.print("eDOMPageNode"); break;
            case eDOMCanvasNode: System.out.print("eDOMCanvasNode"); break;
            case eDOMGlyphNode: System.out.print("eDOMGlyphNode"); break;
            case eDOMRefNode: System.out.print("eDOMRefNode"); break;
            case eDOMJobTkContentNode: System.out.print("eDOMJobTkContentNode"); break;
            case eDOMJobTkNodeNode: System.out.print("eDOMJobTkNodeNode"); break;
            case eDOMJobTkValueNode: System.out.print("eDOMJobTkValueNode"); break;
            case eDOMJobTkGenericNodeNode: System.out.print("eDOMJobTkGenericNodeNode"); break;
            case eDOMJobTkGenericCharacterDataNode: System.out.print("eDOMJobTkGenericCharacterDataNode"); break;
            case eDOMVisualRootNode: System.out.print("eDOMVisualRootNode"); break;
            case eDOMOutlineNode: System.out.print("eDOMOutlineNode"); break;
            case eDOMOutlineEntryNode: System.out.print("eDOMOutlineEntryNode"); break;
            case eDOMAnnotationAppearanceNode: System.out.print("eDOMAnnotationAppearanceNode"); break;
            case eDOMFragmentNode: System.out.print("eDOMFragmentNode"); break;
            case eDOMTileNode: System.out.print("eDOMTileNode"); break;
            case eDOMNodeTypeCnt: System.out.print("eDOMNodeTypeCnt"); break;
        }

        System.out.println();
    }
}
