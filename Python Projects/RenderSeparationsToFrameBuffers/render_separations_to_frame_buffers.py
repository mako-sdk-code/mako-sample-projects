# -----------------------------------------------------------------------
# <copyright file="render_separations_to_framebuffers.py"
#  company="Hybrid Software Helix Ltd">
#  Copyright (C) 2025 Hybrid Software Helix Ltd.
#  All rights reserved.
# </copyright>
# <summary>
#  This example is provided on an "as is" basis and without warranty of any kind.
#  Hybrid Software Helix Ltd. does not warrant or make any representations
#  regarding the use or results of use of this example.
# </summary>
# -----------------------------------------------------------------------

import os
import sys
from jawsmakoIF_python import *

def main():
    try:
        test_file_path = "TestFiles"

        if len(sys.argv) < 5:
            print(f"Usage: {os.path.basename(sys.argv[0])} <source file> <spots to retain> <spots to ignore> <framebuffers=true/false")
            return -1

        input_file = sys.argv[1]
        spots_to_retain = sys.argv[2]
        spots_to_ignore = sys.argv[3]
        render_to_frame_buffers = sys.argv[4]

        if render_to_frame_buffers != "true" and render_to_frame_buffers != "false":
            print("Parameter framebuffers must be true or false not " + render_to_frame_buffers)
            return -1

        # Create Mako instance
        mako = IJawsMako.create("")
        IJawsMako.enableAllFeatures(mako)
        factory = mako.getFactory()

        # Input
        pdf_input = IPDFInput.create(mako)
        assembly = pdf_input.open(os.path.join(test_file_path, input_file))
        page = assembly.getDocument().getPage()
        fixed_page = page.getContent()

        # Set dimensions
        resolution = 576.0
        bounds = FRect(0, 0, page.getWidth(), page.getHeight())
        pixel_width = int(round(bounds.dX / 96.0 * resolution))
        pixel_height = int(round(bounds.dY / 96.0 * resolution))
        depth = 8

        testspace = IDOMColorSpaceDeviceCMYK.create(factory)

        # Create spot lists
        inks = IRendererTransform.findInks(mako, fixed_page)
        component_names = []
        ignore_spot_color_names = CEDLVectString()
        retain_spot_color_names = CEDLVectString()

        num_components = testspace.getNumComponents()
        for i in range(num_components):
            component_names.append(testspace.getColorantName(i))

        for ink in inks.toVector():
            ink_name = ink.getInkName()
            if spots_to_ignore.find(ink_name) >= 0:
                ignore_spot_color_names.append(ink_name)
            elif spots_to_retain.find(ink_name) >= 0:
                retain_spot_color_names.append(ink_name)
                component_names.append(ink_name)

        renderer = IJawsRenderer.create(mako)

        if render_to_frame_buffers == "false":

            # Render using renderSeparations()
            images = renderer.renderSeparations(
                fixed_page,
                depth,
                testspace,
                0,
                bounds,
                pixel_width,
                pixel_height,
                retain_spot_color_names,
                IOptionalContent.Null(),
                eOCEView,
                CEDLVectString(),
                False,
                0,
                ignore_spot_color_names
            )

            # Write output TIFFs
            stem = os.path.splitext(os.path.basename(input_file))[0]

            for j in range(len(component_names)):
                tiff_filename = f"{stem}_regular_{component_names[j]}.tif"
                tiff_path = os.path.join(test_file_path, tiff_filename)
                IDOMTIFFImage.encode(mako, images[j], IOutputStream.createToFile(factory, tiff_path))

        else:
            # Create buffers (one per component)
            num_channels = len(component_names)
            buffers = []
            frame_buffers = CEDLVectCFrameBufferInfo()

            for _ in range(num_channels):
                buf = bytearray(pixel_width * pixel_height * depth)
                buffers.append(buf)

                fb_info = IJawsRenderer.CFrameBufferInfo()
                fb_info.bufferOfs = 0
                fb_info.rowStride = pixel_width
                fb_info.pixelStride = 0
                frame_buffers.append(fb_info)

            # Render using renderSeparationsToFrameBuffers
            renderer.renderSeparationsToFrameBuffers(
                fixed_page,
                depth,
                True,
                pixel_width,
                pixel_height,
                testspace,
                buffers,
                frame_buffers,
                0,  # antiAliased
                bounds,
                CEDLVectWString([name for name in retain_spot_color_names]),
                IOptionalContent.Null(),
                eOCEView,
                CEDLVectWString(),  # processColorNames
                False,  # alphaGeneration
                0,  # bandMemorySize
                CEDLVectWString([name for name in ignore_spot_color_names])
            )

            # Write output TIFFs
            stem = os.path.splitext(os.path.basename(input_file))[0]

            for j in range(num_channels):
                tiff_filename = f"{stem}_frameBuffer_{component_names[j]}.tif"
                tiff_path = os.path.join(test_file_path, tiff_filename)

                writer_pair = IDOMTIFFImage.createWriterAndImage(
                    mako,
                    IDOMColorSpaceDeviceGray.create(factory),
                    pixel_width,
                    pixel_height,
                    depth,
                    96.0, 96.0,
                    IDOMTIFFImage.eTCAuto,
                    IDOMTIFFImage.eTPNone,
                    eIECNone,
                    False,
                    IInputStream.createFromFile(factory, os.path.join(test_file_path, input_file)),
                    IOutputStream.createToFile(factory, tiff_path)
                )

                frame_writer = writer_pair.frameWriter
                for y in range(pixel_height):
                    start = y * pixel_width
                    scanline = bytes(buffers[j][start:start + pixel_width])
                    frame_writer.writeScanLine(scanline)
                frame_writer.flushData()

        print("Rendering complete.")

    except MakoException as e:
        print(f"MakoException: {e.m_errorCode} - {e.m_msg}")
        return 1
    except Exception as e:
        print(f"Exception: {e}")
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
