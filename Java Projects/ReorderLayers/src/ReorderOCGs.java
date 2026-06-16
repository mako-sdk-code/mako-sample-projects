/* -----------------------------------------------------------------------
 * <copyright file="ReorderOCGs.java" company="Hybrid Software Helix Ltd">
 *  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  Example showing how to list and change the Z-order of Optional Content
 *  Groups (layers) in a PDF using Mako Java bindings.
 * </summary>
 * -----------------------------------------------------------------------
 */

import com.globalgraphics.JawsMako.jawsmakoIF.*;

public class ReorderOCGs {
    public static void main(String[] args) throws Exception {
        if (args.length != 2) {
            System.err.println("Usage: ReorderOCGs <input.pdf> <output.pdf>");
            System.exit(1);
        }

        String inputFile = args[0];
        String outputFile = args[1];

        // 1) Create Mako instance
        IJawsMako jawsMako = IJawsMako.create();
        IJawsMako.enableAllFeatures(jawsMako);

        // 2) Open the PDF
        IPDFInput pdfInput = IPDFInput.create(jawsMako);
        IDocumentAssembly assembly = pdfInput.open(inputFile);
        IDocument document = assembly.getDocument();

        // 3) Get Optional Content (layers)
        IOptionalContent optionalContent = document.getOptionalContent();
        if (optionalContent == null) {
            System.out.println("This document has no Optional Content Groups (layers).");
            return;
        }

        // Get all the OCGs in the document
        CEDLVectIOptionalContentGroup groups = optionalContent.getGroups();
        System.out.printf("Found %d optional content groups.%n", groups.size());

        // Get the default configuration, which defines the Z-order
        IOptionalContentConfiguration config = optionalContent.getDefaultConfiguration();

        // Retrieve the current order (list of references)
        CEDLVectOrderEntry order = config.getOrder();

        // 4) Print current order
        System.out.println("\nCurrent layer order:");
        for (int i = 0; i < order.size(); i++) {
            IOptionalContentConfiguration.COrderEntry entry = order.getitem(i);
            if (entry.getIsGroup()) {
                IOptionalContentGroupReference ref = entry.getGroupRef();
                for (int g = 0; g < groups.size(); g++) {
                    IOptionalContentGroup group = groups.getitem(g);
                    if (group.getReference().equals(ref)) {
                        System.out.printf("  %d: %s%n", i, group.getName());
                    }
                }
            }
        }

        // 5) Move the bottom layer (last in Z-order) to the top
        if (order.size() > 1) {
            IOptionalContentConfiguration.COrderEntry lastEntry = order.getitem(order.size() - 1);
            order.erase(order.size() - 1);
            order.insert(0, lastEntry);
            config.setOrder(order);

            System.out.println("\nNew layer order:");
            for (int i = 0; i < order.size(); i++) {
                IOptionalContentConfiguration.COrderEntry entry = order.getitem(i);
                if (entry.getIsGroup()) {
                    IOptionalContentGroupReference ref = entry.getGroupRef();
                    for (int g = 0; g < groups.size(); g++) {
                        IOptionalContentGroup group = groups.getitem(g);
                        if (group.getReference().equals(ref)) {
                            System.out.printf("  %d: %s%n", i, group.getName());
                        }
                    }
                }
            }
        } else {
            System.out.println("Only one layer found; no reordering performed.");
        }

        // 6) Save to new PDF
        IPDFOutput pdfOutput = IPDFOutput.create(jawsMako);
        pdfOutput.writeAssembly(assembly, outputFile);

        System.out.println("\nSaved reordered PDF as: " + outputFile);
    }
}
