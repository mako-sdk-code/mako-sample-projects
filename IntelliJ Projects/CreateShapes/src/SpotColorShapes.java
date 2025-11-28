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

            // Create some spot color brushes
            var spotColorCyan = makeSeparationColor(factory, "LABCyan", new double[] { 62.0, -44.0, -50.0 });
            var spotColorMagenta = makeSeparationColor(factory, "LABMagenta", new double[] { 52.0, 81.0, -7.0 });
            var spotColorYellow = makeSeparationColor(factory, "LABYellow", new double[] { 95.0, -6.0, 96.0 });
            var spotColorBlack = makeSeparationColor(factory, "LABBlack", new double[] { 12.0, 2.0, 0.0 });
            var labCyan = IDOMSolidColorBrush.create(factory, spotColorCyan);
            var labMagenta = IDOMSolidColorBrush.create(factory, spotColorMagenta);
            var labYellow = IDOMSolidColorBrush.create(factory, spotColorYellow);
            var labBlack = IDOMSolidColorBrush.create(factory, spotColorBlack);

            // Create an 'All' spot color space, a color and a brush to draw with
            var spotColorAll = makeSeparationColor(factory, "All", new double[] { 1.0, 1.0, 1.0, 1.0 });
            var all = IDOMSolidColorBrush.create(factory, spotColorAll);

            // Create a 'Rot' spot color
            var spotColorRot = makeSeparationColor(factory, "Rot", new double[] { 0.0, 1.0, 1.0, 0.0 });
            var rot = IDOMSolidColorBrush.create(factory, spotColorRot);

            // Create a 'Blau' spot color
            var spotColorBlau = makeSeparationColor(factory, "Blau", new double[] { 1.0, 1.0, 0.0, 0.0 });
            var blau = IDOMSolidColorBrush.create(factory, spotColorBlau);

            // Create a 'Grun' spot color
            var spotColorGrun = makeSeparationColor(factory, "Grun", new double[] { 1.0, 0.0, 1.0, 0.0 });
            var grun = IDOMSolidColorBrush.create(factory, spotColorGrun);

            // Draw rows of shapes
            FPoint masterBoxSize = new FPoint(80, 80);
            FPoint origin = new FPoint(140, 75);

            drawRow(fixedPage, factory, origin, masterBoxSize, cyan, magenta, yellow, black, ShapeType.Box);
            origin.setY(origin.getY() + 200);
            drawRow(fixedPage, factory, origin, masterBoxSize, labCyan, labMagenta, labYellow, labBlack, ShapeType.Ellipse);
            origin.setY(origin.getY() + 200);
            drawRow(fixedPage, factory, origin, masterBoxSize, rot, grun, blau, all, ShapeType.Hexagon);
            origin.setY(origin.getY() + 200);
            drawRow(fixedPage, factory, origin, masterBoxSize, cyan, magenta, yellow, black, ShapeType.Polygon, 8, 22.5);
            origin.setY(origin.getY() + 200);
            drawRow(fixedPage, factory, origin, masterBoxSize, cyan, magenta, yellow, black, ShapeType.Target);

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
        drawRow(page, factory, origin, masterBoxSize, cyan, magenta, yellow, black, type, 0, 0.0);
    }

    static void drawRow(IDOMFixedPage page, IEDLClassFactory factory, FPoint origin, FPoint masterBoxSize,
                        IDOMBrush cyan, IDOMBrush magenta, IDOMBrush yellow, IDOMBrush black,
                        ShapeType type, int sides, double angle) {

        IDOMBrush[] brushes = new IDOMBrush[]{cyan, magenta, yellow, black};
        FPoint boxSize = new FPoint(masterBoxSize.getX(), masterBoxSize.getY());
        FPoint start = new FPoint(origin.getX(), origin.getY());

        for (IDOMBrush brush : brushes) {
            FRect strokeBox = new FRect(start.getX(), start.getY(), boxSize.getX(), boxSize.getY());
            var strokeGeom = createGeometry(factory, strokeBox, type, sides, angle);
            page.appendChild(IDOMPathNode.createStroked(factory, strokeGeom, brush));

            FRect fillBox = new FRect(strokeBox.getX(), strokeBox.getY() + boxSize.getY() * 1.1, boxSize.getX(), boxSize.getY());
            var fillGeom = createGeometry(factory, fillBox, type, sides, angle);
            page.appendChild(IDOMPathNode.createFilled(factory, fillGeom, brush));

            start.setX(start.getX() + boxSize.getX() * 1.1);
            boxSize.setX(boxSize.getX() * 1.25);
        }
    }

    static IDOMPathGeometry createGeometry(IEDLClassFactory factory, FRect box, ShapeType type, int sides, double angle) {
        switch (type) {
            case Ellipse:
                return IDOMPathGeometry.createEllipse(factory, box);
            case Hexagon:
                return IDOMPathGeometry.createPolygon(factory, box, 6, angle);
            case Polygon:
                return IDOMPathGeometry.createPolygon(factory, box, (short)sides, angle);
            case Target:
                return createTarget(factory, box, 8, angle);
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

    static IDOMColor makeSeparationColor(IEDLClassFactory factory, String name, double[] representation) {
        var colorant = new IDOMColorSpaceDeviceN.CColorantInfo(name, new CEDLVectDouble(representation));
        var colorants = new CEDLVectColorantInfo();
        colorants.append(colorant);

        var alternate = representation.length == 3 ? IDOMColorSpaceLAB.create(factory, 0.9642f, 1.0f, 0.8249f,
                0.0f, 0.0f, 0.0f, -128.0f, 127.0f, -128.0f, 127.0f) : IDOMColorSpaceDeviceCMYK.create(factory);

        var colorSpace = IDOMColorSpaceDeviceN.create(factory, colorants, alternate);
        return IDOMColor.create(factory, colorSpace, 1.0, 1.0);
    }
}
