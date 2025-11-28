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

# Create a fixed page A5 landscape.
pagewidth = 793.7
pageheight = 561.1
fixedpage = IDOMFixedPage.create(factory, pagewidth, pageheight)

# Now create some DOM for the page contents.

# Find a font
font: IDOMFont = jawsmako.findFont('Arial Bold')

# Create a blue brush
hcolorspace = IDOMColorSpacesRGB.create(factory)
bluecolor = IDOMColor.create(factory, hcolorspace, 1.0, 0.0, 0.0, 1.0)
solidBrush = IDOMSolidColorBrush.create(factory, bluecolor)

# Create glyphs
message = 'Mako Python API'
glyphs = IDOMGlyphs.create(factory, message, 72, FPoint(10, pageheight / 2),
                           solidBrush, font[0], 0)

# And add to the page
fixedpage.appendChild(glyphs.toIDOMNode())

# Assign content to a page
page.setContent(fixedpage)

# Now we can write this to a PDF
dir_path = os.path.dirname(os.path.realpath(__file__))
IPDFOutput.create(jawsmako).writeAssembly(assembly, dir_path + '/TestPython.pdf')

print('File created.')
