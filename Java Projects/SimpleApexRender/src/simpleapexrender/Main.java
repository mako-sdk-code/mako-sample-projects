/* --------------------------------------------------------------------------------
 *  <copyright file="Main.java" company="Hybrid Software Helix Ltd">
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

public class Main {
    public static void main(String[] args) {

        var jawsMako = IJawsMako.create();
        var factory = jawsMako.getFactory();
        IJawsMako.enableAllFeatures(jawsMako);
        var input = IPDFInput.create(jawsMako);
        var assembly = input.open("Cheshire Cat.pdf");
        var document = assembly.getDocument();
        var page = document.getPage(0);
        var fixedPage = page.edit();

        // TODO

        // Now we can write this to a PDF
        IPDFOutput.create(jawsMako).writeAssembly(assembly, "TestJava.pdf");

        System.out.println("File created.");
    }
}