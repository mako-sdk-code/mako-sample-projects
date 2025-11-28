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

using System;
using JawsMako;

namespace SpotColorShapes
{
    class Program
    {
        static void Main(string[] args)
        {
            try
            {
                using var mako = IJawsMako.create();
                IJawsMako.enableAllFeatures(mako);
                var factory = mako.getFactory();

                using var assembly = IDocumentAssembly.create(mako);
                using var document = IDocument.create(mako);
                assembly.appendDocument(document);
                using var page = IPage.create(mako);
                document.appendPage(page);
                using var fixedPage = IDOMFixedPage.create(mako);
                page.setContent(fixedPage);

                // Create some process color brushes
                var cyan = IDOMSolidColorBrush.createSolidCmyk(mako, 1.0f, 0.0f, 0.0f, 0.0f);
                var magenta = IDOMSolidColorBrush.createSolidCmyk(mako, 0.0f, 1.0f, 0.0f, 0.0f);
                var yellow = IDOMSolidColorBrush.createSolidCmyk(mako, 0.0f, 0.0f, 1.0f, 0.0f);
                var black = IDOMSolidColorBrush.createSolidCmyk(mako, 0.0f, 0.0f, 0.0f, 1.0f);

                // Create some spot color brushes
                var spotColorCyan = MakeSeparationColor(mako, "LABCyan", new double[] { 62.0, -44.0, -50.0 });
                var spotColorMagenta = MakeSeparationColor(mako, "LABMagenta", new double[] { 52.0, 81.0, -7.0 });
                var spotColorYellow = MakeSeparationColor(mako, "LABYellow", new double[] { 95.0, -6.0, 96.0 });
                var spotColorBlack = MakeSeparationColor(mako, "LABBlack", new double[] { 12.0, 2.0, 0.0 });
                var labCyan = IDOMSolidColorBrush.create(mako, spotColorCyan);
                var labMagenta = IDOMSolidColorBrush.create(mako, spotColorMagenta);
                var labYellow = IDOMSolidColorBrush.create(mako, spotColorYellow);
                var labBlack = IDOMSolidColorBrush.create(mako, spotColorBlack);

                // Create an 'All' spot color space, a color and a brush to draw with
                var spotColorAll = MakeSeparationColor(mako, "All", new double[] { 1.0, 1.0, 1.0, 1.0 });
                var all = IDOMSolidColorBrush.create(mako, spotColorAll);

                // Create a 'Rot' spot color
                var spotColorRot = MakeSeparationColor(mako, "Rot", new double[] { 0.0, 1.0, 1.0, 0.0 });
                var rot = IDOMSolidColorBrush.create(mako, spotColorRot);

                // Create a 'Blau' spot color
                var spotColorBlau = MakeSeparationColor(mako, "Blau", new double[] { 1.0, 1.0, 0.0, 0.0 });
                var blau = IDOMSolidColorBrush.create(mako, spotColorBlau);

                // Create a 'Grun' spot color
                var spotColorGrun = MakeSeparationColor(mako, "Grun", new double[] { 1.0, 0.0, 1.0, 0.0 });
                var grun = IDOMSolidColorBrush.create(mako, spotColorGrun);

                // Draw rows of shapes exactly as in C++ (stroke/fill pairs separated vertically)
                var masterBoxSize = new FPoint(80, 80);
                FPoint origin = new FPoint(140, 75);

                DrawRow(fixedPage, mako, origin, masterBoxSize, cyan, magenta, yellow, black, ShapeType.Box);
                origin.y += 200;
                DrawRow(fixedPage, mako, origin, masterBoxSize, labCyan, labMagenta, labYellow, labBlack, ShapeType.Ellipse);
                origin.y += 200;
                DrawRow(fixedPage, mako, origin, masterBoxSize, rot, grun, blau, all, ShapeType.Hexagon);
                origin.y += 200;
                DrawRow(fixedPage, mako, origin, masterBoxSize, cyan, magenta, yellow, black, ShapeType.Polygon, 8, 22.5);
                origin.y += 200;
                DrawRow(fixedPage, mako, origin, masterBoxSize, cyan, magenta, yellow, black, ShapeType.Target);

                using var pdf = IPDFOutput.create(mako);
                pdf.writeAssembly(assembly, "test.pdf");
            }
            catch (MakoException e)
            {
                Console.WriteLine($"MakoException: {e.m_msg}");
            }
            catch (Exception e)
            {
                Console.WriteLine($"Exception: {e.Message}");
            }
        }

        enum ShapeType { Box, Ellipse, Hexagon, Polygon, Target }

        static void DrawRow(IDOMFixedPage page, IJawsMako mako, FPoint origin, FPoint masterBoxSize,
                            IDOMBrush cyan, IDOMBrush magenta, IDOMBrush yellow, IDOMBrush black,
                            ShapeType type, int sides = 0, double angle = 0)
        {
            var brushes = new[] { cyan, magenta, yellow, black };
            var boxSize = new FPoint(masterBoxSize.x, masterBoxSize.y);
            var start = new FPoint(origin.x, origin.y);

            foreach (var brush in brushes)
            {
                // Draw stroked version first
                var strokeBox = new FRect(start.x, start.y, boxSize.x, boxSize.y);
                var strokeGeom = CreateGeometry(mako, strokeBox, type, sides, angle);
                page.appendChild(IDOMPathNode.createStroked(mako, strokeGeom, brush));

                // Now draw filled version below it
                var fillBox = new FRect(strokeBox.x, strokeBox.y + boxSize.y * 1.1, boxSize.x, boxSize.y);
                var fillGeom = CreateGeometry(mako, fillBox, type, sides, angle);
                page.appendChild(IDOMPathNode.createFilled(mako, fillGeom, brush));

                // Move to next column and expand width like C++
                start.x += boxSize.x * 1.1;
                boxSize.x *= 1.25;
            }
        }

        static IDOMPathGeometry CreateGeometry(IJawsMako mako, FRect box, ShapeType type, int sides, double angle)
        {
            return type switch
            {
                ShapeType.Ellipse => IDOMPathGeometry.createEllipse(mako, box),
                ShapeType.Hexagon => IDOMPathGeometry.createPolygon(mako, box, 6, angle),
                ShapeType.Polygon => IDOMPathGeometry.createPolygon(mako, box, (uint)sides, angle),
                ShapeType.Target => CreateTarget(mako, box, 8, angle),
                _ => IDOMPathGeometry.create(mako, box)
            };
        }

        static IDOMPathGeometry CreateTarget(IJawsMako mako, FRect box, int nLines, double rotation)
        {
            double angle = rotation * Math.PI / 180.0;
            double incr = 2.0 * Math.PI / nLines;
            double radiusX = box.dX / 2.0;
            double radiusY = box.dY / 2.0;
            double centerX = box.x + radiusX;
            double centerY = box.y + radiusY;

            var builder = IDOMPathGeometryBuilder.create(mako);
            var center = new FPoint(centerX, centerY);
            builder.moveTo(center);

            for (int i = 0; i < nLines; i++)
            {
                double endX = centerX + radiusX * Math.Cos(angle);
                double endY = centerY + radiusY * Math.Sin(angle);
                builder.lineTo(new FPoint(endX, endY));
                builder.moveTo(center);
                angle += incr;
            }
            builder.close();
            return builder.createGeometry(mako, IDOMPathGeometry.eFillRule.eFRNonZero);
        }

        static IDOMColor MakeSeparationColor(IJawsMako mako, string name, double[] representation)
        {
            var colorant = new IDOMColorSpaceDeviceN.CColorantInfo(name, new CEDLVectDouble(representation));
            var colorants = new CEDLVectColorantInfo();
            colorants.append(colorant);

            IDOMColorSpace alternate = representation.Length == 3 ? IDOMColorSpaceLAB.create(mako, 0.9642f, 1.0f, 0.8249f,
                                                     0.0f, 0.0f, 0.0f, -128.0f, 127.0f,
                                                     -128.0f, 127.0f) : IDOMColorSpaceDeviceCMYK.create(mako);

            var colorSpace = IDOMColorSpaceDeviceN.create(mako, colorants, alternate);
            return IDOMColor.create(mako, colorSpace, 1.0, 1.0);
        }
    }
}