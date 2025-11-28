""" -----------------------------------------------------------------------
 * <copyright file="main.py" company="Hybrid Software Helix Ltd">
 *  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 """

import os
from jawsmakoIF_python import *

# Instantiate Mako
jawsmako = IJawsMako.create("")
IJawsMako.enableAllFeatures(jawsmako)
factory = jawsmako.getFactory()

# Create an assembly, document and page
assembly = IDocumentAssembly.create(jawsmako)
document = IDocument.create(jawsmako)
assembly.appendDocument(document)
page = IPage.create(jawsmako)
document.appendPage(page)
fixedpage = IDOMFixedPage.create(factory)

# TODO

# Assign content to a page
page.setContent(fixedpage)

# Now we can write this to a PDF
dir_path = os.path.dirname(os.path.realpath(__file__))
IPDFOutput.create(jawsmako).writeAssembly(assembly, dir_path + '/TestPython.pdf')

print('File created.')