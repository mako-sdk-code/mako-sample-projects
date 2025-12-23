# -----------------------------------------------------------------------
# <copyright file="spot_color_shapes.py" company="Hybrid Software Helix Ltd">
#  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
# </copyright>
# <summary>
#  This example is provided on an "as is" basis and without warranty of any kind.
#  Hybrid Software Helix Ltd. does not warrant or make any representations
#  regarding the use or results of use of this example.
# </summary>
# -----------------------------------------------------------------------

import math
from jawsmakoIF_python import *


class ShapeType:
    Box = 0
    Ellipse = 1
    Hexagon = 2
    Polygon = 3
    Target = 4


def main():
    try:
        # Instantiate Mako
        jaws_mako = IJawsMako.create("")
        IJawsMako.enableAllFeatures(jaws_mako)
        factory = jaws_mako.getFactory()

        # Create the document and page structure
        assembly = IDocumentAssembly.create(jaws_mako)
        document = IDocument.create(jaws_mako)
        assembly.appendDocument(document)
        page = IPage.create(jaws_mako)
        document.appendPage(page)
        fixed_page = IDOMFixedPage.create(factory)
        page.setContent(fixed_page)

        # Create some process color brushes
        cyan = IDOMSolidColorBrush.createSolidCmyk(factory,1.0, 0.0, 0.0, 0.0)
        magenta = IDOMSolidColorBrush.createSolidCmyk(factory, 0.0, 1.0, 0.0, 0.0)
        yellow = IDOMSolidColorBrush.createSolidCmyk(factory, 0.0, 0.0, 1.0, 0.0)
        black = IDOMSolidColorBrush.createSolidCmyk(factory, 0.0, 0.0, 0.0, 1.0)

        # Create some spot color brushes with types LAB, ICCBased and DeviceN

        # Create an LAB colorspace with D65 white point and -128 to 127 ranges for a and b values
        labColorSpace = IDOMColorSpaceLAB.create(factory, 0.9504, 1.0, 1.0888, 0.0, 0.0, 0.0, -128, 127, -128, 127)

        # Create a device CMYK colorspace from an ICC profile
        iccBasedColorSpace = IDOMColorSpaceICCBased.create(
            factory, IDOMICCProfile.create(
                factory, IInputStream.createFromFile(
                    factory, "C:\\Windows\\System32\\spool\\drivers\\color\\WebCoatedFOGRA28.icc")))

        # Create a LAB color
        pantoneBlue072C_lab = IDOMColor.create(factory, labColorSpace, 1.0, 17.64, 43.0, -76.0)

        # Make a copy and convert to ICC space
        pantoneBlue072C_fogra = IDOMColor.fromRCObject(pantoneBlue072C_lab.clone(factory))
        pantoneBlue072C_fogra.setColorSpace(iccBasedColorSpace, eRelativeColorimetric, eBPCDefault, factory)

        # Create a DeviceN color
        pantoneBlue072C_spot = make_deviceN_color(factory, "PANTONE BLUE 072 C", CEDLVectDouble({ 17.64, 43.0, -76.0 }), labColorSpace)

        # Create an All spot color
        allColor = make_deviceN_color(factory, "All", CEDLVectDouble({ 1.0, 1.0, 1.0, 1.0 }), IDOMColorSpaceDeviceCMYK.create(factory))

        # Create spot color brushes
        labBrush = IDOMSolidColorBrush.create(factory, pantoneBlue072C_lab)
        iccBrush = IDOMSolidColorBrush.create(factory, pantoneBlue072C_fogra)
        deviceNBrush = IDOMSolidColorBrush.create(factory, pantoneBlue072C_spot)
        allBrush = IDOMSolidColorBrush.create(factory, allColor)

        # Draw rows of shapes
        master_box_size = FPoint(90, 90)
        boxSize = FPoint(master_box_size)
        origin = FPoint(85, 85)
        start = FPoint(origin)

        # Draw Cyan box
        box = FRect(start.x, start.y, boxSize.x, boxSize.y)
        fixed_page.appendChild(IDOMPathNode.createFilled(factory, IDOMPathGeometry.create(factory, box), cyan))
        start.x += boxSize.x

        # Draw Magenta box
        box = FRect(start.x, start.y, boxSize.x, boxSize.y)
        fixed_page.appendChild(IDOMPathNode.createFilled(factory, IDOMPathGeometry.create(factory, box), magenta))
        start.x += boxSize.x

        # Draw Yellow box
        box = FRect(start.x, start.y, boxSize.x, boxSize.y)
        fixed_page.appendChild(IDOMPathNode.createFilled(factory, IDOMPathGeometry.create(factory, box), yellow))
        start.x += boxSize.x

        # Draw Black box
        box = FRect(start.x, start.y, boxSize.x, boxSize.y)
        fixed_page.appendChild(IDOMPathNode.createFilled(factory, IDOMPathGeometry.create(factory, box), black))
        start.x += boxSize.x

        # Draw LAB box
        box = FRect(start.x, start.y, boxSize.x, boxSize.y)
        fixed_page.appendChild(IDOMPathNode.createFilled(factory, IDOMPathGeometry.create(factory, box), labBrush))
        start.x += boxSize.x

        # Draw ICC box
        box = FRect(start.x, start.y, boxSize.x, boxSize.y)
        fixed_page.appendChild(IDOMPathNode.createFilled(factory, IDOMPathGeometry.create(factory, box), iccBrush))
        start.x += boxSize.x

        # Draw DeviceN box
        box = FRect(start.x, start.y, boxSize.x, boxSize.y)
        fixed_page.appendChild(IDOMPathNode.createFilled(factory, IDOMPathGeometry.create(factory, box), deviceNBrush))
        start.x += boxSize.x

        # Draw All box
        box = FRect(start.x, start.y, boxSize.x, boxSize.y)
        fixed_page.appendChild(IDOMPathNode.createFilled(factory, IDOMPathGeometry.create(factory, box), allBrush))

        # Draw a border in All
        strokeWidth = 20
        box = FRect(origin.x - strokeWidth / 2.0, origin.y - strokeWidth / 2.0, (boxSize.x * 7) + strokeWidth, boxSize.y + strokeWidth)
        fixed_page.appendChild(IDOMPathNode.createStroked(factory, IDOMPathGeometry.create(factory, box), allBrush, FMatrix(),
         IDOMPathGeometry.Null(), strokeWidth))

        # Draw some other shapes below
        origin.y += 160

        draw_row(fixed_page, factory, origin, master_box_size, cyan, magenta, yellow, black, ShapeType.Ellipse)
        origin.y += 200
        draw_row(fixed_page, factory, origin, master_box_size, labBrush, iccBrush, deviceNBrush, allBrush, ShapeType.Hexagon)
        origin.y += 200
        draw_row(fixed_page, factory, origin, master_box_size, cyan, magenta, yellow, black,
                 ShapeType.Polygon, sides=8, angle=22.5)
        origin.y += 200
        draw_row(fixed_page, factory, origin, master_box_size, labBrush, iccBrush, deviceNBrush, allBrush, ShapeType.Target)

        # Write to PDF
        pdf = IPDFOutput.create(jaws_mako)
        pdf.writeAssembly(assembly, "test.pdf")

    except Exception as e:
        print("Exception:", str(e))


def draw_row(page, factory, origin, master_box_size, cyan, magenta, yellow, black, shape_type, sides=0, angle=0.0, lines=0):
    brushes = [cyan, magenta, yellow, black]
    box_size = FPoint(master_box_size.x, master_box_size.y)
    start = FPoint(origin.x, origin.y)

    for brush in brushes:
        stroke_box = FRect(start.x, start.y, box_size.x, box_size.y)
        stroke_geom = create_geometry(factory, stroke_box, shape_type, sides, angle, lines)
        page.appendChild(IDOMPathNode.createStroked(factory, stroke_geom, brush))

        fill_box = FRect(stroke_box.x, stroke_box.y + box_size.y * 1.1, box_size.x, box_size.y)
        fill_geom = create_geometry(factory, fill_box, shape_type, sides, angle, lines + 4)
        page.appendChild(IDOMPathNode.createFilled(factory, fill_geom, brush))

        start.x += box_size.x * 1.2
        box_size.x *= 1.3

        lines += 1


def create_geometry(factory, box, shape_type, sides, angle, lines):
    if shape_type == ShapeType.Ellipse:
        return IDOMPathGeometry.createEllipse(factory, box)
    elif shape_type == ShapeType.Hexagon:
        return IDOMPathGeometry.createPolygon(factory, box, 6, angle)
    elif shape_type == ShapeType.Polygon:
        return IDOMPathGeometry.createPolygon(factory, box, int(sides), angle)
    elif shape_type == ShapeType.Target:
        return create_target(factory, box, lines, angle)
    else:
        return IDOMPathGeometry.create(factory, box)


def create_target(factory, box, n_lines, rotation):
    angle = math.radians(rotation)
    incr = 2.0 * math.pi / n_lines
    radius_x = box.dX / 2.0
    radius_y = box.dY / 2.0
    center_x = box.x + radius_x
    center_y = box.y + radius_y

    builder = IDOMPathGeometryBuilder.create(factory)
    center = FPoint(center_x, center_y)
    builder.moveTo(center)

    for _ in range(n_lines):
        end_x = center_x + radius_x * math.cos(angle)
        end_y = center_y + radius_y * math.sin(angle)
        builder.lineTo(FPoint(end_x, end_y))
        builder.moveTo(center)
        angle += incr

    builder.close()
    return builder.createGeometry(factory, IDOMPathGeometry.eFRNonZero)


def make_deviceN_color(factory, name, representation, alternate):
    # Create a vector of colorants with one entry
    colorants = CEDLVectColorantInfo()
    colorant = IDOMColorSpaceDeviceN.CColorantInfo(name, representation)
    colorants.append(colorant)

    # Create the DeviceN space
    color_space = IDOMColorSpaceDeviceN.create(factory, colorants, alternate)
    # Return the new spot color
    return IDOMColor.create(factory, color_space, 1.0, 1.0)


if __name__ == "__main__":
    main()
