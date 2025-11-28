/* --------------------------------------------------------------------------------
 *  <copyright file="WalkTree.cs" company="Hybrid Software Helix Ltd">
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

namespace WalkTreeExample;

internal class WalkTree
{
    static int Main()
    {
        try
        {
            var testFilepath = @"..\..\..\..\TestFiles\";

            var mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);

            // Input
            var pdfInput = IPDFInput.create(mako);
            using var assembly = pdfInput.open(testFilepath + "Cheshire Cat.pdf");
            using var inFixedPage = assembly.getDocument().getPage()
                .getContent();

            //void VisitNodeHandler(IDOMNode node) => DumpNodeInfo(node);
            //WalkTreeCallback callback = new NodeTreeCallbackDelegate(VisitNodeHandler);

            WalkTreeCallback callback = new NodeTreeCallback();

            inFixedPage.walkTree(callback.getCallbackFunc(), callback.getPriv(), false, true);

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

    private class NodeTreeCallback : WalkTreeCallback
    {
        public override bool visitNode(IDOMNode node)
        {
            DumpNodeInfo(node);
            return true;
        }
    }

    // ReSharper disable once UnusedType.Local
    private class NodeTreeCallbackDelegate(NodeTreeCallbackDelegate.visitNodeDelegate visitNodeHandler) : WalkTreeCallback
    {
        public delegate void visitNodeDelegate(IDOMNode node);

        public override bool visitNode(IDOMNode node)
        {
            if (visitNodeHandler == null)
                return false;
            visitNodeHandler(node);
            return true;
        }
    }

    static void DumpNodeInfo(IDOMNode node)
    {
        IDOMNode p = node;
        while (p.getParentNode() is not null)
        {
            Console.Write("  ");
            p = p.getParentNode();
        }

        switch (node.getNodeType())
        {
            case eDOMNodeType.eDOMFixedPageNode:
                Console.Write("eDOMFixedPageNode");
                break;
            case eDOMNodeType.eDOMGroupNode:
                Console.Write("eDOMGroupNode");
                break;
            case eDOMNodeType.eDOMCharPathGroupNode:
                Console.Write("eDOMCharPathGroupNode");
                break;
            case eDOMNodeType.eDOMTransparencyGroupNode:
                Console.Write("eDOMTransparencyGroupNode");
                break;
            case eDOMNodeType.eDOMGlyphsNode:
                Console.Write("eDOMGlyphsNode");
                break;
            case eDOMNodeType.eDOMPathNode:
                Console.Write("eDOMPathNode");
                break;
            case eDOMNodeType.eDOMFormNode:
                Console.Write("eDOMFormNode");
                break;
            case eDOMNodeType.eDOMFormInstanceNode:
                Console.Write("eDOMFormInstanceNode");
                break;
            case eDOMNodeType.eDOMContentRootNode:
                Console.Write("eDOMContentRootNode");
                break;
            case eDOMNodeType.eDOMDocumentSequenceNode:
                Console.Write("eDOMDocumentSequenceNode");
                break;
            case eDOMNodeType.eDOMDocumentNode:
                Console.Write("eDOMDocumentNode");
                break;
            case eDOMNodeType.eDOMFixedDocumentNode:
                Console.Write("eDOMFixedDocumentNode");
                break;
            case eDOMNodeType.eDOMPageNode:
                Console.Write("eDOMPageNode");
                break;
            case eDOMNodeType.eDOMCanvasNode:
                Console.Write("eDOMCanvasNode");
                break;
            case eDOMNodeType.eDOMGlyphNode:
                Console.Write("eDOMGlyphNode");
                break;
            case eDOMNodeType.eDOMRefNode:
                Console.Write("eDOMRefNode");
                break;
            case eDOMNodeType.eDOMJobTkContentNode:
                Console.Write("eDOMJobTkContentNode");
                break;
            case eDOMNodeType.eDOMJobTkNodeNode:
                Console.Write("eDOMJobTkNodeNode");
                break;
            case eDOMNodeType.eDOMJobTkValueNode:
                Console.Write("eDOMJobTkValueNode");
                break;
            case eDOMNodeType.eDOMJobTkGenericNodeNode:
                Console.Write("eDOMJobTkGenericNodeNode");
                break;
            case eDOMNodeType.eDOMJobTkGenericCharacterDataNode:
                Console.Write("eDOMJobTkGenericCharacterDataNode");
                break;
            case eDOMNodeType.eDOMVisualRootNode:
                Console.Write("eDOMVisualRootNode");
                break;
            case eDOMNodeType.eDOMOutlineNode:
                Console.Write("eDOMOutlineNode");
                break;
            case eDOMNodeType.eDOMOutlineEntryNode:
                Console.Write("eDOMOutlineEntryNode");
                break;
            case eDOMNodeType.eDOMAnnotationAppearanceNode:
                Console.Write("eDOMAnnotationAppearanceNode");
                break;
            case eDOMNodeType.eDOMFragmentNode:
                Console.Write("eDOMFragmentNode");
                break;
            case eDOMNodeType.eDOMTileNode:
                Console.Write("eDOMTileNode");
                break;
            case eDOMNodeType.eDOMNodeTypeCnt:
                Console.Write("eDOMNodeTypeCnt");
                break;
        }
        Console.WriteLine();
    }
}