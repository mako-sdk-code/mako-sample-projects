/* -----------------------------------------------------------------------
 * <copyright file="Main.cs" company="Hybrid Software Helix Ltd">
 *  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

using JawsMako;

namespace CreateShapesCS;

class Program
{
    static int Main(string[] args)
    {
        if (args.Length != 1)
        {
            Console.Error.WriteLine($"Usage: {AppDomain.CurrentDomain.FriendlyName} <icc-profile>");
            return 1;
        }

        try
        {
            var iccProfilePath = args[0];
            if (!File.Exists(iccProfilePath))
                throw new FileNotFoundException("ICC profile was not found.", iccProfilePath);

            using var mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);

            using var assembly = IDocumentAssembly.create(mako);
            using var document = IDocument.create(mako);
            assembly.appendDocument(document);
            using var page = IPage.create(mako);
            document.appendPage(page);
            using var fixedPage = IDOMFixedPage.create(mako);
            page.setContent(fixedPage);

            // Create some process color brushes
            using var cyan = IDOMSolidColorBrush.createSolidCmyk(mako, 1.0f, 0.0f, 0.0f, 0.0f);
            using var magenta = IDOMSolidColorBrush.createSolidCmyk(mako, 0.0f, 1.0f, 0.0f, 0.0f);
            using var yellow = IDOMSolidColorBrush.createSolidCmyk(mako, 0.0f, 0.0f, 1.0f, 0.0f);
            using var black = IDOMSolidColorBrush.createSolidCmyk(mako, 0.0f, 0.0f, 0.0f, 1.0f);

            // Create some spot color brushes with types LAB, ICCBased and DeviceN

            // Create an LAB colorspace with D65 white point and -128 to 127 ranges for a and b values
            using var labColorSpace = IDOMColorSpaceLAB.create(mako, 0.9504f, 1.0f, 1.0888f, 0.0f, 0.0f, 0.0f, -128, 127, -128, 127);

            // Create a device CMYK colorspace from an ICC profile
            using var iccBasedColorSpace = IDOMColorSpaceICCBased.create(
                mako, IDOMICCProfile.create(
                    mako, IInputStream.createFromFile(
                        mako, iccProfilePath)));

            // Create a LAB color
            using var pantoneBlue072C_lab = IDOMColor.create(mako, labColorSpace, 1.0, 17.64, 43.0, -76.0);

            // Make a copy and convert to ICC space
            using var pantoneBlue072C_fogra = IDOMColor.fromRCObject(pantoneBlue072C_lab.clone(mako));
            pantoneBlue072C_fogra.setColorSpace(iccBasedColorSpace, eRenderingIntent.eRelativeColorimetric, eBlackPointCompensation.eBPCDefault, mako);

            // Create a DeviceN color
            using var pantoneBlue072C_spot = MakeDeviceNColor(mako, "PANTONE BLUE 072 C", new CEDLVectDouble(new double[] { 17.64, 43.0, -76.0 }), labColorSpace);

            // Create an All spot color
            using var allColor = MakeDeviceNColor(mako, "All", new CEDLVectDouble(new[] { 1.0, 1.0, 1.0, 1.0 }), IDOMColorSpaceDeviceCMYK.create(mako));

            // Create spot color brushes
            using var labBrush = IDOMSolidColorBrush.create(mako, pantoneBlue072C_lab);
            using var iccBrush = IDOMSolidColorBrush.create(mako, pantoneBlue072C_fogra);
            using var deviceNBrush = IDOMSolidColorBrush.create(mako, pantoneBlue072C_spot);
            using var allBrush = IDOMSolidColorBrush.create(mako, allColor);

            // Set box size and origin
            using var masterBoxSize = new FPoint(90, 90);
            using var boxSize = new FPoint(masterBoxSize);
            using var origin = new FPoint(85, 85);
            using var start = new FPoint(origin);

            // Draw Cyan box
            var box = new FRect(start.x, start.y, boxSize.x, boxSize.y);
            fixedPage.appendChild(IDOMPathNode.createFilled(mako, IDOMPathGeometry.create(mako, box), cyan));
            start.x += boxSize.x;

            // Draw Magenta box
            box = new FRect(start.x, start.y, boxSize.x, boxSize.y);
            fixedPage.appendChild(IDOMPathNode.createFilled(mako, IDOMPathGeometry.create(mako, box), magenta));
            start.x += boxSize.x;

            // Draw Yellow box
            box = new FRect(start.x, start.y, boxSize.x, boxSize.y);
            fixedPage.appendChild(IDOMPathNode.createFilled(mako, IDOMPathGeometry.create(mako, box), yellow));
            start.x += boxSize.x;

            // Draw Black box
            box = new FRect(start.x, start.y, boxSize.x, boxSize.y);
            fixedPage.appendChild(IDOMPathNode.createFilled(mako, IDOMPathGeometry.create(mako, box), black));
            start.x += boxSize.x;

            // Draw LAB box
            box = new FRect(start.x, start.y, boxSize.x, boxSize.y);
            fixedPage.appendChild(IDOMPathNode.createFilled(mako, IDOMPathGeometry.create(mako, box), labBrush));
            start.x += boxSize.x;

            // Draw ICC box
            box = new FRect(start.x, start.y, boxSize.x, boxSize.y);
            fixedPage.appendChild(IDOMPathNode.createFilled(mako, IDOMPathGeometry.create(mako, box), iccBrush));
            start.x += boxSize.x;

            // Draw DeviceN box
            box = new FRect(start.x, start.y, boxSize.x, boxSize.y);
            fixedPage.appendChild(IDOMPathNode.createFilled(mako, IDOMPathGeometry.create(mako, box), deviceNBrush));
            start.x += boxSize.x;

            // Draw a border in All
            const uint strokeWidth = 20;
            box = new FRect(origin.x - strokeWidth / 2.0, origin.y - strokeWidth / 2.0, (boxSize.x * 7) + strokeWidth, boxSize.y + strokeWidth);
            fixedPage.appendChild(IDOMPathNode.createStroked(mako, IDOMPathGeometry.create(mako, box), allBrush, new FMatrix(),
                IDOMPathGeometry.Null(), strokeWidth));

            // Draw some other shapes below
            origin.y += 180;

            DrawRow(fixedPage, mako, origin, masterBoxSize, [cyan, magenta, yellow, black], ShapeType.Ellipse);
            origin.y += 200;
            DrawRow(fixedPage, mako, origin, masterBoxSize, [labBrush, iccBrush, deviceNBrush, allBrush], ShapeType.Hexagon);
            origin.y += 200;
            DrawRow(fixedPage, mako, origin, masterBoxSize, [cyan, magenta, yellow, black], ShapeType.Polygon, 8, 22.5);
            origin.y += 200;
            DrawRow(fixedPage, mako, origin, masterBoxSize, [labBrush, iccBrush, deviceNBrush, allBrush], ShapeType.Target, 0, 0.0, 3);

            using var pdf = IPDFOutput.create(mako);
            pdf.writeAssembly(assembly, "test.pdf");
        }
        catch (MakoException e)
        {
            Console.WriteLine($"MakoException: {e.m_msg}");
            return 1;
        }
        catch (Exception e)
        {
            Console.WriteLine($"Exception: {e.Message}");
            return 1;
        }

        return 0;
    }

    private enum ShapeType { Ellipse, Hexagon, Polygon, Target }

    private static void DrawRow(IDOMFixedPage page, IJawsMako mako, FPoint origin, FPoint masterBoxSize,
        IDOMBrush[] brushes,
        ShapeType type, int sides = 0, double angle = 0, int lines = 0)
    {
        var boxSize = new FPoint(masterBoxSize.x, masterBoxSize.y);
        var start = new FPoint(origin.x, origin.y);

        foreach (var brush in brushes)
        {
            // Draw stroked version first
            var strokeBox = new FRect(start.x, start.y, boxSize.x, boxSize.y);
            var strokeGeom = CreateGeometry(mako, strokeBox, type, sides, angle, lines);
            page.appendChild(IDOMPathNode.createStroked(mako, strokeGeom, brush));

            // Now draw filled version below it
            var fillBox = new FRect(strokeBox.x, strokeBox.y + boxSize.y * 1.1, boxSize.x, boxSize.y);
            var fillGeom = CreateGeometry(mako, fillBox, type, sides, angle, lines + 4);
            page.appendChild(IDOMPathNode.createFilled(mako, fillGeom, brush));

            // Move to next column and expand width 
            start.x += boxSize.x * 1.2;
            boxSize.x *= 1.3;

            lines += 1;
        }
    }

    private static IDOMPathGeometry CreateGeometry(IJawsMako mako, FRect box, ShapeType type, int sides, double angle, int lines)
    {
        return type switch
        {
            ShapeType.Ellipse => IDOMPathGeometry.createEllipse(mako, box),
            ShapeType.Hexagon => IDOMPathGeometry.createPolygon(mako, box, 6, angle),
            ShapeType.Polygon => IDOMPathGeometry.createPolygon(mako, box, (uint)sides, angle),
            ShapeType.Target => CreateTarget(mako, box, lines, angle),
            _ => IDOMPathGeometry.create(mako, box)
        };
    }

    private static IDOMPathGeometry CreateTarget(IJawsMako mako, FRect box, int nLines, double rotation)
    {
        var angle = rotation * Math.PI / 180.0;
        var incr = 2.0 * Math.PI / nLines;
        var radiusX = box.dX / 2.0;
        var radiusY = box.dY / 2.0;
        var centerX = box.x + radiusX;
        var centerY = box.y + radiusY;

        var builder = IDOMPathGeometryBuilder.create(mako);
        var center = new FPoint(centerX, centerY);
        builder.moveTo(center);

        for (var i = 0; i < nLines; i++)
        {
            var endX = centerX + radiusX * Math.Cos(angle);
            var endY = centerY + radiusY * Math.Sin(angle);
            builder.lineTo(new FPoint(endX, endY));
            builder.moveTo(center);
            angle += incr;
        }
        builder.close();
        return builder.createGeometry(mako, IDOMPathGeometry.eFillRule.eFRNonZero);
    }

    private static IDOMColor MakeDeviceNColor(IJawsMako mako, string name, CEDLVectDouble representation, IDOMColorSpace alternate)
    {
        // Create a vector of colorants with one entry
        var colorants = new CEDLVectColorantInfo();
        var colorant = new IDOMColorSpaceDeviceN.CColorantInfo(name, representation);
        colorants.append(colorant);

        // Create the DeviceN space
        var colorSpace = IDOMColorSpaceDeviceN.create(mako, colorants, alternate);

        // Return the new spot color
        return IDOMColor.create(mako, colorSpace, 1.0, 1.0);
    }
}