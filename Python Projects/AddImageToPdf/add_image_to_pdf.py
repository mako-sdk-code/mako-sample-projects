""" -----------------------------------------------------------------------
 * <copyright file="AddImageToPdf.py" company="Hybrid Software Helix Ltd">
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
from pathlib import Path
from jawsmakoIF_python import *

TEST_FILES_PATH = str(Path(__file__).resolve().parents[2] / "TestFiles") + os.sep
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
fixedPage = IDOMFixedPage.create(factory)

# Load image from file
image = IDOMPNGImage.create(factory, IInputStream.createFromFile(factory, TEST_FILES_PATH + "makologo.png"))

# Get image attributes
imageFrame = image.getImageFrame(factory)
width = imageFrame.getWidth()
height = imageFrame.getHeight()

# Add image to fixedPage
imageNode = IDOMPathNode.createImage(factory, image, FRect(0.0, 0.0, width, height))
fixedPage.appendChild(imageNode)

# Assign content to a page
page.setContent(fixedPage)

# Now we can write this to a PDF
dir_path = os.path.dirname(os.path.realpath(__file__))
IPDFOutput.create(jawsmako).writeAssembly(assembly, dir_path + '/TestPython.pdf')

print('File created.')
