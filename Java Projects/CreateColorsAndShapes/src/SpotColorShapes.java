/* -----------------------------------------------------------------------
 * <copyright file="SpotColorShapes.java" company="Hybrid Software Helix Ltd">
 *  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

import com.globalgraphics.JawsMako.jawsmakoIF.*;
import java.lang.Math;

public class SpotColorShapes {

    enum ShapeType { Box, Ellipse, Hexagon, Polygon, Target }

    public static void main(String[] args) {
        try {
            var mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);
            var factory = mako.getFactory();

            var assembly = IDocumentAssembly.create(mako);
            var document = IDocument.create(mako);
            assembly.appendDocument(document);
            var page = IPage.create(mako);
            document.appendPage(page);
            var fixedPage = IDOMFixedPage.create(factory);
            page.setContent(fixedPage);

            // Create some process color brushes
            var cyan = IDOMSolidColorBrush.createSolidCmyk(factory, 1.0f, 0.0f, 0.0f, 0.0f);
            var magenta = IDOMSolidColorBrush.createSolidCmyk(factory, 0.0f, 1.0f, 0.0f, 0.0f);
            var yellow = IDOMSolidColorBrush.createSolidCmyk(factory, 0.0f, 0.0f, 1.0f, 0.0f);
            var black = IDOMSolidColorBrush.createSolidCmyk(factory, 0.0f, 0.0f, 0.0f, 1.0f);

            // Create some spot color brushes with types LAB, ICCBased and DeviceN

            // Create an LAB colorspace with D65 white point and -128 to 127 ranges for a and b values
            var labColorSpace = IDOMColorSpaceLAB.create(factory, 0.9504f, 1.0f, 1.0888f, 0.0f, 0.0f, 0.0f, -128, 127, -128, 127);

            // Create a device CMYK colorspace from an ICC profile
            var iccBasedColorSpace = IDOMColorSpaceICCBased.create(
                factory, IDOMICCProfile.create(
                    factory, IInputStream.createFromFile(
                        factory, "C:\\Windows\\System32\\spool\\drivers\\color\\WebCoatedFOGRA28.icc")));

            // Create a LAB color
            var pantoneBlue072C_lab = IDOMColor.create(factory, labColorSpace, 1.0, 17.64, 43.0, -76.0);

            // Make a copy and convert to ICC space
            IDOMColor pantoneBlue072C_fogra = IDOMColor.fromRCObject(pantoneBlue072C_lab.clone(factory));
            pantoneBlue072C_fogra.setColorSpace(iccBasedColorSpace, eRenderingIntent.eRelativeColorimetric, eBlackPointCompensation.eBPCDefault, factory);

            // Create a DeviceN color
            var pantoneBlue072C_spot = MakeDeviceNColor(factory, "PANTONE BLUE 072 C", new CEDLVectDouble(new double[] { 17.64, 43.0, -76.0 }), labColorSpace);

            // Create an All spot color
            var allColor = MakeDeviceNColor(factory, "All", new CEDLVectDouble(new double[] { 1.0, 1.0, 1.0, 1.0 }), IDOMColorSpaceDeviceCMYK.create(factory));

            // Create spot color brushes
            var labBrush = IDOMSolidColorBrush.create(factory, pantoneBlue072C_lab);
            var iccBrush = IDOMSolidColorBrush.create(factory, pantoneBlue072C_fogra);
            var deviceNBrush = IDOMSolidColorBrush.create(factory, pantoneBlue072C_spot);
            var allBrush = IDOMSolidColorBrush.create(factory, allColor);

            // Draw rows of shapes
            FPoint masterBoxSize = new FPoint(90, 90);
            var boxSize = new FPoint(masterBoxSize);
            var origin = new FPoint(85, 85);
            var start = new FPoint(origin);

            // Draw Cyan box
            var box = new FRect(start.getX(), start.getY(), boxSize.getX(), boxSize.getY());
            fixedPage.appendChild(IDOMPathNode.createFilled(factory, IDOMPathGeometry.create(factory, box), cyan));
            start.setX(start.getX() + boxSize.getX());

            // Draw Magenta box
            box = new FRect(start.getX(), start.getY(), boxSize.getX(), boxSize.getY());
            fixedPage.appendChild(IDOMPathNode.createFilled(factory, IDOMPathGeometry.create(factory, box), magenta));
            start.setX(start.getX() + boxSize.getX());

            // Draw Yellow box
            box = new FRect(start.getX(), start.getY(), boxSize.getX(), boxSize.getY());
            fixedPage.appendChild(IDOMPathNode.createFilled(factory, IDOMPathGeometry.create(factory, box), yellow));
            start.setX(start.getX() + boxSize.getX());

            // Draw Black box
            box = new FRect(start.getX(), start.getY(), boxSize.getX(), boxSize.getY());
            fixedPage.appendChild(IDOMPathNode.createFilled(factory, IDOMPathGeometry.create(factory, box), black));
            start.setX(start.getX() + boxSize.getX());

            // Draw LAB box
            box = new FRect(start.getX(), start.getY(), boxSize.getX(), boxSize.getY());
            fixedPage.appendChild(IDOMPathNode.createFilled(factory, IDOMPathGeometry.create(factory, box), labBrush));
            start.setX(start.getX() + boxSize.getX());

            // Draw ICC box
            box = new FRect(start.getX(), start.getY(), boxSize.getX(), boxSize.getY());
            fixedPage.appendChild(IDOMPathNode.createFilled(factory, IDOMPathGeometry.create(factory, box), iccBrush));
            start.setX(start.getX() + boxSize.getX());

            // Draw DeviceN box
            box = new FRect(start.getX(), start.getY(), boxSize.getX(), boxSize.getY());
            fixedPage.appendChild(IDOMPathNode.createFilled(factory, IDOMPathGeometry.create(factory, box), deviceNBrush));
            start.setX(start.getX() + boxSize.getX());

            // Draw a border in All
            long strokeWidth = 20;
            box = new FRect(origin.getX() - strokeWidth / 2.0, origin.getY() - strokeWidth / 2.0, (boxSize.getX() * 7) + strokeWidth, boxSize.getY() + strokeWidth);
            fixedPage.appendChild(IDOMPathNode.createStroked(factory, IDOMPathGeometry.create(factory, box), allBrush, new FMatrix(),
                IDOMPathGeometry.Null(), strokeWidth));

            // Draw some other shapes below
            origin.setY(origin.getY() + 180);

            drawRow(fixedPage, factory, origin, masterBoxSize, cyan, magenta, yellow, black, ShapeType.Ellipse);
            origin.setY(origin.getY() + 200);
            drawRow(fixedPage, factory, origin, masterBoxSize, labBrush, iccBrush, deviceNBrush, allBrush, ShapeType.Hexagon);
            origin.setY(origin.getY() + 200);
            drawRow(fixedPage, factory, origin, masterBoxSize, cyan, magenta, yellow, black, ShapeType.Polygon, 8, 22.5, 0);
            origin.setY(origin.getY() + 200);
            drawRow(fixedPage, factory, origin, masterBoxSize, labBrush, iccBrush, deviceNBrush, allBrush, ShapeType.Target, 0, 0.0, 3);

            var pdf = IPDFOutput.create(mako);
            pdf.writeAssembly(assembly, "test.pdf");
        }
        catch (Exception e) {
            System.out.println("Exception: " + e.getMessage());
        }
    }

    static void drawRow(IDOMFixedPage page, IEDLClassFactory factory, FPoint origin, FPoint masterBoxSize,
                        IDOMBrush cyan, IDOMBrush magenta, IDOMBrush yellow, IDOMBrush black,
                        ShapeType type) {
        drawRow(page, factory, origin, masterBoxSize, cyan, magenta, yellow, black, type, 0, 0.0, 3);
    }

    static void drawRow(IDOMFixedPage page, IEDLClassFactory factory, FPoint origin, FPoint masterBoxSize,
                        IDOMBrush cyan, IDOMBrush magenta, IDOMBrush yellow, IDOMBrush black,
                        ShapeType type, int sides, double angle, int lines) {

        IDOMBrush[] brushes = new IDOMBrush[]{cyan, magenta, yellow, black};
        FPoint boxSize = new FPoint(masterBoxSize.getX(), masterBoxSize.getY());
        FPoint start = new FPoint(origin.getX(), origin.getY());

        for (IDOMBrush brush : brushes) {
            FRect strokeBox = new FRect(start.getX(), start.getY(), boxSize.getX(), boxSize.getY());
            var strokeGeom = createGeometry(factory, strokeBox, type, sides, angle, lines);
            page.appendChild(IDOMPathNode.createStroked(factory, strokeGeom, brush));

            FRect fillBox = new FRect(strokeBox.getX(), strokeBox.getY() + boxSize.getY() * 1.1, boxSize.getX(), boxSize.getY());
            var fillGeom = createGeometry(factory, fillBox, type, sides, angle, lines + 4);
            page.appendChild(IDOMPathNode.createFilled(factory, fillGeom, brush));

            start.setX(start.getX() + boxSize.getX() * 1.2);
            boxSize.setX(boxSize.getX() * 1.3);

            lines += 1;
        }
    }

    static IDOMPathGeometry createGeometry(IEDLClassFactory factory, FRect box, ShapeType type, int sides, double angle, int lines) {
        switch (type) {
            case Ellipse:
                return IDOMPathGeometry.createEllipse(factory, box);
            case Hexagon:
                return IDOMPathGeometry.createPolygon(factory, box, 6, angle);
            case Polygon:
                return IDOMPathGeometry.createPolygon(factory, box, (short)sides, angle);
            case Target:
                return createTarget(factory, box, lines, angle);
            default:
                return IDOMPathGeometry.create(factory, box);
        }
    }

    static IDOMPathGeometry createTarget(IEDLClassFactory factory, FRect box, int nLines, double rotation) {
        double angle = Math.toRadians(rotation);
        double incr = 2.0 * Math.PI / nLines;
        double radiusX = box.getDX() / 2.0;
        double radiusY = box.getDY() / 2.0;
        double centerX = box.getX() + radiusX;
        double centerY = box.getY() + radiusY;

        var builder = IDOMPathGeometryBuilder.create(factory);
        FPoint center = new FPoint(centerX, centerY);
        builder.moveTo(center);

        for (int i = 0; i < nLines; i++) {
            double endX = centerX + radiusX * Math.cos(angle);
            double endY = centerY + radiusY * Math.sin(angle);
            builder.lineTo(new FPoint(endX, endY));
            builder.moveTo(center);
            angle += incr;
        }

        builder.close();
        return builder.createGeometry(factory, IDOMPathGeometry.eFillRule.eFRNonZero);
    }

    static IDOMColor MakeDeviceNColor(IEDLClassFactory factory, String name, CEDLVectDouble representation, IDOMColorSpace alternate) {

        // Create a vector of colorants with one entry
        var colorants = new CEDLVectColorantInfo();
        var colorant = new IDOMColorSpaceDeviceN.CColorantInfo(name, representation);
        colorants.append(colorant);

        // Create the DeviceN space
        var colorSpace = IDOMColorSpaceDeviceN.create(factory, colorants, alternate);

        // Return the new spot color
        return IDOMColor.create(factory, colorSpace, 1.0, 1.0);
    }
}
