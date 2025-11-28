/* -----------------------------------------------------------------------
 * <copyright file="OverprintMethods.java" company="Hybrid Software Helix Ltd">
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
import com.globalgraphics.JawsMako.jawsmakoIF.jawsmakoIF.*;

public class OverprintMethods {

    public static void main(String[] args) {
        String testFilePath = "TestFiles/";

        try {
            // Create the Mako instance
            IJawsMako mako = IJawsMako.create("", "");
            IJawsMako.enableAllFeatures(mako);

            // Open the input PDF
            IInput input = IInput.create(mako, eFileFormat.eFFPDF);
            IDocumentAssembly assembly = input.open(testFilePath + "CMYK_Circles 1.pdf");
            IDocument document = assembly.getDocument();

            // Iterate through all pages
            for (int pageIndex = 0; pageIndex < document.getNumPages(); pageIndex++) {
                IPage page = document.getPage(pageIndex);
                IDOMFixedPage fixedPage = page.edit();

                // Find all path nodes on the page
                CEDLVectIDOMNode pathNodes = fixedPage.findChildrenOfType(eDOMNodeType.eDOMPathNode, true);

                // Apply overprint to each path node
                for (int i = 0; i < pathNodes.size(); i++) {
                    IDOMPathNode path = IDOMPathNode.fromRCObject(pathNodes.getitem(i));
                    if (path != null) {
                        // Apply overprint fill using the new API
                        path.setFillOverprints(true);
                        // Optionally apply stroke overprint
                        // path.setStrokeOverprints(true);
                        path.setOverprintMode(true);
                    }
                }
            }

            // Write to a new PDF file
            IPDFOutput pdfOutput = IPDFOutput.create(mako);
            pdfOutput.writeAssembly(assembly, "test.pdf");

        } catch (Exception e) {
            System.err.println("Exception thrown: " + e.getMessage());
            System.exit(1);
        }
    }
}
