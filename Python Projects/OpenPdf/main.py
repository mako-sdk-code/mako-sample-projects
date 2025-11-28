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
jawsmako = IJawsMako.create()
IJawsMako.enableAllFeatures(jawsmako)
factory = jawsmako.getFactory()

# Open an assembly and get the document and page
input = IInput.create(jawsmako, eFFPDF)
assembly = input.open("Cheshire Cat.pdf")
document = assembly.getDocument()
page = document.getPage(0)
fixedPage = page.edit()

# TODO

# Now we can write this to a PDF
dir_path = os.path.dirname(os.path.realpath(__file__))
IPDFOutput.create(jawsmako).writeAssembly(assembly, dir_path + '/TestPython.pdf', IProgressTick.Null())

print('File created.')