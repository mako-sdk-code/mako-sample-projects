#
# -----------------------------------------------------------------------
# <copyright file="custom_spot_merge.py" company="Hybrid Software Helix Ltd">
#  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
# </copyright>
# <summary>
#  This example is provided on an "as is" basis and without warranty of any kind.
#  Hybrid Software Helix Ltd. does not warrant or make any representations
#  regarding the use or results of use of this example.
# </summary>
# -----------------------------------------------------------------------
#

import math
import sys
import os
from pathlib import Path

import jawsmakoIF_python
from jawsmakoIF_python import *

TEST_FILES_PATH = str(Path(__file__).resolve().parents[2] / "TestFiles") + os.sep

def custom_spot_merge():
    # Adjust to your test files folder

    try:
        # Instantiate Mako
        mako = IJawsMako.create("")
        IJawsMako.enableAllFeatures(mako)
        factory = mako.getFactory()

        # Load the document
        pdf_input = IPDFInput.create(mako)
        doc_asm = pdf_input.open(TEST_FILES_PATH + "Robots Plus Process Colors.pdf")
        document = doc_asm.getDocument()

        for page_index in range(document.getNumPages()):
            fixed_page = document.getPage(page_index).getContent()

            # Page bounds & raster geometry
            bounds = FRect(0, 0, fixed_page.getWidth(), fixed_page.getHeight())
            resolution = 150.0
            pixel_width = int(round(bounds.dX / 96.0 * resolution))
            pixel_height = int(round(bounds.dY / 96.0 * resolution))

            # Colorspace must be CMYK for spot merging
            cmyk = IDOMColorSpaceDeviceCMYK.create(factory)

            # Find inks and build spot list
            inks = IRendererTransform.findInks(mako, fixed_page)
            spots = IRendererTransform.inkInfoToColorantInfo(mako, inks, cmyk)

            spot_names = CEDLVectWString()
            for i in range(spots.size()):
                spot_names.append(spots.getitem(i).name)

            num_process = cmyk.getNumComponents()  # 4
            num_spots = int(spots.size())
            num_buffers = num_process + num_spots

            # Prepare frame buffers
            buffers = []
            fb = CEDLVectCFrameBufferInfo()

            for i in range(num_buffers):
                # allocate pixel buffer for this plane
                buf = bytearray(pixel_width * pixel_height)
                buffers.append(buf)

                info = IJawsRenderer.CFrameBufferInfo()
                info.bufferOfs = 0
                info.rowStride = pixel_width
                info.pixelStride = 0
                fb.append(info)

            # Render true separations
            renderer = IJawsRenderer.create(mako)
            renderer.renderSeparationsToFrameBuffers(
                fixed_page,
                8,
                True,
                pixel_width,
                pixel_height,
                cmyk,
                buffers,
                fb,
                0,
                bounds,
                spot_names
            )

            # Create writer and image
            pair = IDOMRawImage.createWriterAndImage(
                mako,
                cmyk,
                pixel_width,
                pixel_height,
                8,
                resolution,
                resolution
            )
            frame_writer = pair.frameWriter

            # Spot components to merge
            components = []
            for i in range(num_spots):
                spot = spots.getitem(i)
                vals = spot.components
                components.append([vals.getitem(c) for c in range(num_process)])

            # Merge each spot buffer with process values
            inv255 = 1.0 / 255.0
            scanline = bytearray(pixel_width * num_process)

            for y in range(pixel_height):
                row_start = y * pixel_width
                for x in range(pixel_width):
                    for c in range(num_process):
                        idx = x * num_process + c
                        scanline[idx] = buffers[c][row_start + x]
                        for i in range(num_spots):
                            spot_val = buffers[num_process + i][row_start + x] * inv255
                            current_val = scanline[idx] * inv255
                            new_val = 1.0 - (1.0 - components[i][c] * spot_val) * (1.0 - current_val)
                            scanline[idx] = int(new_val * 255.0 + 0.5)
                frame_writer.writeScanLine(scanline)

            frame_writer.flushData()

            # Convert to RGB and save as JPEG
            rgb = IDOMColorSpaceDeviceRGB.create(factory)
            cc = IDOMImageColorConverterFilter.create(
                factory,
                rgb,
                eRelativeColorimetric,
                eBPCDefault
            )
            filtered = IDOMFilteredImage.create(factory, pair.domImage, cc)

            out_jpeg = f"output_{page_index}.jpg"
            IDOMJPEGImage.encode(mako, filtered, IOutputStream.createToFile(factory, out_jpeg))

            print(f"Wrote: {out_jpeg}")

    except MakoException as ex:
        print(f"MakoException {ex.m_errorCode}: {ex.m_msg}")
        return 1
    except Exception as ex:
        print(f"Error: {ex}")
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(custom_spot_merge())
