# -----------------------------------------------------------------------
# <copyright file="MakoRecolorImageWithSpot.py" company="Hybrid Software Helix Ltd">
#  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
# </copyright>
# <summary>
#  Recolor Image With Spot Example
#  Demonstrates how to paint an image with a spot color
#
#  This example is provided on an "as is" basis and without warranty of any kind.
#  Hybrid Software Helix Ltd. does not warrant or make any representations
#  regarding the use or results of use of this example.
# </summary>
# -----------------------------------------------------------------------

from jawsmakoIF_python import *

def main():
    try:
        test_filepath = "TestFiles/"

        # Instantiate Mako
        jaws_mako = IJawsMako.create("")
        IJawsMako.enableAllFeatures(jaws_mako)
        factory = jaws_mako.getFactory()

        # Create a spot color and corresponding solid color brush
        rubine_red = make_separation_color(jaws_mako, "Pantone Rubine Red C", [43.92, 79.0, 13.0])
        rubine_red_brush = IDOMSolidColorBrush.create(jaws_mako.getFactory(), rubine_red)

        # Load an image from disk
        image = IDOMPNGImage.create(jaws_mako.getFactory(),
                                         IInputStream.createFromFile(jaws_mako.getFactory(), test_filepath + "Cheshire Cat.png"))

        # Get image details
        image_frame = image.getImageFrame(jaws_mako.getFactory())
        image_colorspace = image_frame.getColorSpace()
        image_width = image_frame.getWidth()
        image_height = image_frame.getHeight()
        image_xres = image_frame.getXResolution()
        image_yres = image_frame.getYResolution()
        image_bounds = FRect(0.0, 0.0, image_width / image_xres * 96.0, image_height / image_yres * 96.0)

        # Color convert to grayscale and invert
        image_filters = CEDLVectIDOMImageFilter()
        if image_colorspace.getNumComponents() != 1:
            image_filters.append(
                IDOMImageColorConverterFilter.create(factory,
                                                          IDOMColorSpaceDeviceGray.create(factory),
                                                          ePerceptual,
                                                          eBPCDefault))
        image_filters.append(IDOMImageInverterFilter.create(jaws_mako.getFactory()))

        # Apply filters to create a mask
        mask = IDOMFilteredImage.create(factory, image, image_filters)

        # Create a masked brush
        masked_brush = IDOMMaskedBrush.create(factory, mask, rubine_red_brush, image_bounds, image_bounds)

        # Create a path of suitable size and paint with masked brush
        path = IDOMPathNode.createFilled(factory,
                                              IDOMPathGeometry.create(factory, image_bounds),
                                              masked_brush)

        # Create document assembly, document, page, and fixed page
        assembly = IDocumentAssembly.create(jaws_mako)
        document = IDocument.create(jaws_mako)
        assembly.appendDocument(document)
        page = IPage.create(jaws_mako)
        document.appendPage(page)
        fixed_page = IDOMFixedPage.create(factory)
        page.setContent(fixed_page)

        # Scale the image to fit the page
        path.setRenderTransform(FMatrix(image_bounds, page.getCropBox()))

        # Add to page
        fixed_page.appendChild(path)

        # Save to PDF
        pdf_output = IPDFOutput.create(jaws_mako)
        pdf_output.writeAssembly(assembly,
                                 IOutputStream.createToFile(factory, "Cheshire Cat.pdf"))

    except MakoException as e:
        print(f"Exception thrown: {e.m_errorCode}: {e.m_msg}")
    except Exception as e:
        print(f"Exception thrown: {e}")


def make_separation_color(jaws_mako, name, lab_representation):
    """Define an L*a*b spot color"""
    colorant_info = IDOMColorSpaceDeviceN.CColorantInfo(name, CEDLVectDouble(lab_representation))
    colorants = CEDLVectColorantInfo()
    colorants.append(colorant_info)

    alternate = IDOMColorSpaceLAB.create(jaws_mako.getFactory(), 0.9642, 1.0000, 0.8249, 0.0, 0.0, 0.0, -128.0, 127.0, -128.0, 127.0)
    color_space = IDOMColorSpaceDeviceN.create(jaws_mako.getFactory(), colorants, alternate.toIDOMColorSpace())

    return IDOMColor.createFromVect(jaws_mako.getFactory(), color_space, 1.0, [1.0])


def make_cmyk_separation_color(jaws_mako, name, cmyk_representation):
    """Define a CMYK spot color"""
    colorant_info = IDOMColorSpaceDeviceN.CColorantInfo(name, CEDLVectDouble(cmyk_representation))
    colorants = CEDLVectColorantInfo()
    colorants.append(colorant_info)

    alternate = IDOMColorSpaceDeviceCMYK.create(jaws_mako)
    color_space = IDOMColorSpaceDeviceN.create(jaws_mako.getFactory(), colorants, alternate.toIDOMColorSpace())

    return IDOMColor.createFromVect(jaws_mako.getFactory(), color_space, 1.0, [1.0])


if __name__ == "__main__":
    main()
