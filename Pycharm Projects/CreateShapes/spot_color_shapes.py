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

        # Create some spot color brushes
        spotColorCyan = make_separation_color(factory, "LABCyan", [ 62.0, -44.0, -50.0] )
        spotColorMagenta = make_separation_color(factory, "LABMagenta", [ 52.0, 81.0, -7.0 ])
        spotColorYellow = make_separation_color(factory, "LABYellow", [ 95.0, -6.0, 96.0 ])
        spotColorBlack = make_separation_color(factory, "LABBlack", [ 12.0, 2.0, 0.0 ])
        labCyan = IDOMSolidColorBrush.create(factory, spotColorCyan)
        labMagenta = IDOMSolidColorBrush.create(factory, spotColorMagenta)
        labYellow = IDOMSolidColorBrush.create(factory, spotColorYellow)
        labBlack = IDOMSolidColorBrush.create(factory, spotColorBlack)

         # Create an 'All' spot color space, a color and a brush to draw with
        spotColorAll = make_separation_color(factory, "All", [ 1.0, 1.0, 1.0, 1.0 ])
        all = IDOMSolidColorBrush.create(factory, spotColorAll)

         # Create a 'Rot' spot color
        spotColorRot = make_separation_color(factory, "Rot", [ 0.0, 1.0, 1.0, 0.0 ])
        rot = IDOMSolidColorBrush.create(factory, spotColorRot)

         # Create a 'Blau' spot color
        spotColorBlau = make_separation_color(factory, "Blau", [ 1.0, 1.0, 0.0, 0.0 ])
        blau = IDOMSolidColorBrush.create(factory, spotColorBlau)

        # Create a 'Grun' spot color
        spotColorGrun = make_separation_color(factory, "Grun", [ 1.0, 0.0, 1.0, 0.0 ])
        grun = IDOMSolidColorBrush.create(factory, spotColorGrun)

        # Draw rows of shapes
        master_box_size = FPoint(80, 80)
        origin = FPoint(140, 75)

        draw_row(fixed_page, factory, origin, master_box_size, cyan, magenta, yellow, black, ShapeType.Box)
        origin.y += 200
        draw_row(fixed_page, factory, origin, master_box_size, labCyan, labMagenta, labYellow, labBlack, ShapeType.Ellipse)
        origin.y += 200
        draw_row(fixed_page, factory, origin, master_box_size, rot, grun, blau, all, ShapeType.Hexagon)
        origin.y += 200
        draw_row(fixed_page, factory, origin, master_box_size, cyan, magenta, yellow, black,
                 ShapeType.Polygon, sides=8, angle=22.5)
        origin.y += 200
        draw_row(fixed_page, factory, origin, master_box_size, cyan, magenta, yellow, black, ShapeType.Target)

        # Write to PDF
        pdf = IPDFOutput.create(jaws_mako)
        pdf.writeAssembly(assembly, "test.pdf")

    except Exception as e:
        print("Exception:", str(e))


def draw_row(page, factory, origin, master_box_size, cyan, magenta, yellow, black, shape_type, sides=0, angle=0.0):
    brushes = [cyan, magenta, yellow, black]
    box_size = FPoint(master_box_size.x, master_box_size.y)
    start = FPoint(origin.x, origin.y)

    for brush in brushes:
        stroke_box = FRect(start.x, start.y, box_size.x, box_size.y)
        stroke_geom = create_geometry(factory, stroke_box, shape_type, sides, angle)
        page.appendChild(IDOMPathNode.createStroked(factory, stroke_geom, brush))

        fill_box = FRect(stroke_box.x, stroke_box.y + box_size.y * 1.1, box_size.x, box_size.y)
        fill_geom = create_geometry(factory, fill_box, shape_type, sides, angle)
        page.appendChild(IDOMPathNode.createFilled(factory, fill_geom, brush))

        start.x += box_size.x * 1.1
        box_size.x *= 1.25


def create_geometry(factory, box, shape_type, sides, angle):
    if shape_type == ShapeType.Ellipse:
        return IDOMPathGeometry.createEllipse(factory, box)
    elif shape_type == ShapeType.Hexagon:
        return IDOMPathGeometry.createPolygon(factory, box, 6, angle)
    elif shape_type == ShapeType.Polygon:
        return IDOMPathGeometry.createPolygon(factory, box, int(sides), angle)
    elif shape_type == ShapeType.Target:
        return create_target(factory, box, 8, angle)
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


def make_separation_color(factory, name, representation):
    colorant = IDOMColorSpaceDeviceN.CColorantInfo(name, CEDLVectDouble(representation))
    colorants = CEDLVectColorantInfo()
    colorants.append(colorant)

    alternate = IDOMColorSpaceLAB.create(factory,
                                         0.9642, 1.0, 0.8249,
                                         0.0, 0.0, 0.0,
                                         -128.0, 127.0, -128.0, 127.0) if len(representation) == 3 else IDOMColorSpaceDeviceCMYK.create(factory)
    color_space = IDOMColorSpaceDeviceN.create(factory, colorants, alternate)
    return IDOMColor.create(factory, color_space, 1.0, 1.0)


if __name__ == "__main__":
    main()
