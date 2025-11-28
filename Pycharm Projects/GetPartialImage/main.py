#
# -----------------------------------------------------------------------
# <copyright file="mako_partial_image.py" company="Hybrid Software Helix Ltd">
#  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
# </copyright>
# <summary>
#  This example is provided on an "as is" basis and without warranty of any kind.
#  Hybrid Software Helix Ltd. does not warrant or make any representations
#  regarding the use or results of use of this example.
# </summary>
# -----------------------------------------------------------------------
#
from pickletools import uint8

from jawsmakoIF_python import *


def main():
    try:
        # Instantiate Mako
        jawsMako = IJawsMako.create("")
        factory = jawsMako.getFactory()
        IJawsMako.enableAllFeatures(jawsMako)

        test_file_path = "TestFiles/"
        image = IDOMJPEGImage.create(
            factory,
            IInputStream.createFromFile(factory, test_file_path + "WEV_086.JPG")
        )

        # Extract a portion of the image (the kayak area)
        sub_image_rect = FRect(230, 230, 400, 250)
        partial_image = get_partial_image(jawsMako, image, sub_image_rect)

        # Encode the partial image as a PNG
        IDOMPNGImage.encode(
            jawsMako,
            partial_image,
            IOutputStream.createToFile(factory, "JustTheKayak.png")
        )

    except MakoException as e:
        print(f"Mako exception thrown: {e.m_msg}")
    except Exception as e:
        print(f"Exception thrown: {e}")


def get_partial_image(jawsMako: IJawsMako, image: IDOMImage, sub_image_rect: FRect) -> IDOMImage:
    """Extract part of an image described by an FRect where:
       x,y are top-left corner, in pixels
       dX, dY are width and height
    """
    factory = jawsMako.getFactory()
    image_frame = image.getImageFrame(factory)
    bps = image_frame.getBPS()

    # Scale bits per sample to 8 or 16 if needed
    if bps < 8:
        image = IDOMFilteredImage.create(
            jawsMako,
            image,
            IDOMImageBitScalerFilter.create(factory, 8)
        )
        image_frame = image.getImageFrame(jawsMako)
        bps = 8
    elif bps != 8 and bps != 16:
        image = IDOMFilteredImage.create(
            jawsMako,
            image,
            IDOMImageBitScalerFilter.create(factory, 16)
        )
        image_frame = image.getImageFrame(jawsMako)
        bps = 16

    color_space = image_frame.getColorSpace()
    stride = image_frame.getRawBytesPerRow()
    bpp = image_frame.getNumChannels() * bps // 8

    # Ensure the requested rectangle is within bounds
    original_rect = FRect(0.0, 0.0, image_frame.getWidth(), image_frame.getHeight())
    if not original_rect.containsRect(sub_image_rect):
        return IDOMImage.Null()

    # Create temporary compressed reader/writer pair
    temp = jawsMako.getTempStore().createTemporaryReaderWriter()
    in_stream = IInputStream.createFromLz4Compressed(factory, temp.toIInputStream())
    out_stream = IOutputStream.createToLz4Compressed(factory, temp.toIOutputStream())

    image_and_writer = IDOMRawImage.createWriterAndImage(
  jawsMako,
        color_space,
        int(sub_image_rect.dX),
        int(sub_image_rect.dY),
        bps,
        image_frame.getXResolution(),
        image_frame.getYResolution(),
        eIECNone,
        in_stream,
        out_stream
    )

    sub_image = image_and_writer.domImage
    frame_writer = image_and_writer.frameWriter

    row_buffer = bytearray(stride)
    target_row_buffer = bytearray(int(sub_image_rect.dX) * bpp)

    # Skip down to the first row of the sub-image
    image_frame.skipScanLines(int(sub_image_rect.y))

    # Copy the region
    for _ in range(int(sub_image_rect.dY)):
        image_frame.readScanLine(row_buffer, stride)
        start = int(sub_image_rect.x) * bpp
        target_row_buffer[:] = row_buffer[start:start + len(target_row_buffer)]
        frame_writer.writeScanLine(target_row_buffer)

    frame_writer.flushData()
    return sub_image


if __name__ == "__main__":
    main()
