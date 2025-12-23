/* -----------------------------------------------------------------------
 * <copyright file="Main.cpp" company="Hybrid Software Helix Ltd">
 *  Copyright (c) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

#include <iostream>
#include <jawsmako/jawsmako.h>
#include <jawsmako/pdfoutput.h>
#include <edl/idomcolorspace.h>
#include <jawsmako/layout.h>

using namespace JawsMako;
using namespace EDL;

IDOMPathGeometryPtr createHexagon(const IJawsMakoPtr& mako, FRect box, double rotation = 0);
IDOMPathGeometryPtr createTarget(const IJawsMakoPtr& mako, FRect box, double nLines, double rotation);
IDOMColorPtr MakeDeviceNColor(const IJawsMakoPtr& mako, const U8String& name, const CDoubleVect& representation, IDOMColorSpacePtr alternateSpace);

int main()
{
    try
    {
        const auto mako = IJawsMako::create();
        mako->enableAllFeatures(mako);
        const auto assembly = IDocumentAssembly::create(mako);
        const auto document = IDocument::create(mako);
        assembly->appendDocument(document);
        const auto page = IPage::create(mako);
        document->appendPage(page);
        const auto fixedPage = IDOMFixedPage::create(mako);
        page->setContent(fixedPage);

        // Create some process color brushes
        const auto cyan = IDOMSolidColorBrush::createSolidCmyk(mako, 1.0, 0.0, 0.0, 0.0);
        const auto magenta = IDOMSolidColorBrush::createSolidCmyk(mako, 0.0, 1.0, 0.0, 0.0);
        const auto yellow = IDOMSolidColorBrush::createSolidCmyk(mako, 0.0, 0.0, 1.0, 0.0);
        const auto black = IDOMSolidColorBrush::createSolidCmyk(mako, 0.0, 0.0, 0.0, 1.0);

        // Create some spot color brushes with types LAB, ICCBased and DeviceN

        // Create an LAB colorspace with D65 white point and -128 to 127 ranges for a and b values
        const auto labColorSpace = IDOMColorSpaceLAB::create(mako, 0.9504f, 1.0f, 1.0888f, 0.0, 0.0, 0.0, -128, 127, -128, 127);

        // Create a device CMYK colorspace from an ICC profile
        const auto iccBasedColorSpace = IDOMColorSpaceICCBased::create(
             mako, IDOMICCProfile::create(
                mako, IInputStream::createFromFile(
                    mako, R"(C:\Windows\System32\spool\drivers\color\WebCoatedFOGRA28.icc)")));
        
        // Create an LAB color
        const auto pantoneBlue072C_lab = IDOMColor::create(mako, labColorSpace, 1.0, 17.64, 43.0, -76.0);

        // Make a copy and convert to ICC space
        auto pantoneBlue072C_fogra = clone(pantoneBlue072C_lab, mako);
        pantoneBlue072C_fogra->setColorSpace(iccBasedColorSpace, eRelativeColorimetric, eBPCDefault, mako);

        // Create a DeviceN color
        const auto pantoneBlue072C_spot = MakeDeviceNColor(mako, "PANTONE BLUE 072 C", CDoubleVect{ 17.64, 43.0, -76.0 }, labColorSpace);

        // Create an All spot color
        const auto allColor = MakeDeviceNColor(mako, "All", CDoubleVect{ 1.0, 1.0, 1.0, 1.0 }, IDOMColorSpaceDeviceCMYK::create(mako));

        // Create spot color brushes
        const auto labBrush = IDOMSolidColorBrush::create(mako, pantoneBlue072C_lab);
        const auto iccBrush = IDOMSolidColorBrush::create(mako, pantoneBlue072C_fogra);
        const auto deviceNBrush = IDOMSolidColorBrush::create(mako, pantoneBlue072C_spot);
        const auto allBrush = IDOMSolidColorBrush::create(mako, allColor);

        // Draw some boxes
        const auto masterBoxSize = FPoint(90, 90);
        auto boxSize = masterBoxSize;
        auto origin = FPoint(85, 85);
        auto start = origin;

        // Draw Cyan box
        auto box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::create(mako, box), cyan));
        start.x += boxSize.x;

        // Draw Magenta box
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::create(mako, box), magenta));
        start.x += boxSize.x;

        // Draw Yellow box
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::create(mako, box), yellow));
        start.x += boxSize.x;

        // Draw Black box
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::create(mako, box), black));
        start.x += boxSize.x;

        // Draw LAB box 
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::create(mako, box), labBrush));
        start.x += boxSize.x;

        // Draw ICC box
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::create(mako, box), iccBrush));
        start.x += boxSize.x;

        // Draw DeviceN box
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::create(mako, box), deviceNBrush));
        start.x += boxSize.x;
        
        // Draw a border in All
        const uint32 strokeWidth = 20;
        box = FRect(origin.x - strokeWidth / 2.0, origin.y - strokeWidth / 2.0, (boxSize.x * 7) + strokeWidth, boxSize.y + strokeWidth);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, IDOMPathGeometry::create(mako, box), allBrush, FMatrix(),
            IDOMPathGeometryPtr(), strokeWidth));

        // Draw some other shapes below
        boxSize = masterBoxSize;
        origin.y += 180;
        start = origin;

        // Draw Cyan circle
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, IDOMPathGeometry::createEllipse(mako, box), cyan));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::createEllipse(mako, box), cyan));

        // Draw Magenta circle
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.3;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, IDOMPathGeometry::createEllipse(mako, box), magenta));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::createEllipse(mako, box), magenta));

        // Draw Yellow circle
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.3;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, IDOMPathGeometry::createEllipse(mako, box), yellow));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::createEllipse(mako, box), yellow));

        // Draw Black circle
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.3;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, IDOMPathGeometry::createEllipse(mako, box), black));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::createEllipse(mako, box), black));

        // Draw some hexagons
        boxSize = masterBoxSize;
        origin.y += 200;
        start = origin;

        // Draw LAB hexagon
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createHexagon(mako, box), labBrush));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, createHexagon(mako, box), labBrush));

        // Draw ICC hexagon
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.3;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createHexagon(mako, box), iccBrush));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, createHexagon(mako, box), iccBrush));

        // Draw DeviceN hexagon
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.3;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createHexagon(mako, box), deviceNBrush));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, createHexagon(mako, box), deviceNBrush));

        // Draw All color hexagon
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.3;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createHexagon(mako, box), allBrush));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, createHexagon(mako, box), allBrush));

        // Draw some polygons
        auto sides = 8;
        auto angle = 22.5;
        boxSize = masterBoxSize;
        origin.y += 200;
        start = origin;

        // Draw Cyan polygon
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, IDOMPathGeometry::createPolygon(mako, box, sides, angle), cyan));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::createPolygon(mako, box, sides, angle), cyan));

        // Draw Magenta polygon
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.3;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, IDOMPathGeometry::createPolygon(mako, box, sides, angle), magenta));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::createPolygon(mako, box, sides, angle), magenta));

        // Draw Yellow polygon
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.3;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, IDOMPathGeometry::createPolygon(mako, box, sides, angle), yellow));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::createPolygon(mako, box, sides, angle), yellow));

        // Draw Black polygon
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.3;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, IDOMPathGeometry::createPolygon(mako, box, sides, angle), black));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::createPolygon(mako, box, sides, angle), black));

        // ----- Draw some targets -----
        
        auto lines = 3;
        angle = 0.0;
        boxSize = masterBoxSize;
        origin.y += 200;
        start = origin;

        // Draw LAB polygon
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createTarget(mako, box, lines, angle), labBrush));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createTarget(mako, box, lines + 4, angle), labBrush));

        // Draw ICC polygon
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.3;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        lines += 1;
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createTarget(mako, box, lines, angle), iccBrush));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createTarget(mako, box, lines + 4, angle), iccBrush));

        // Draw DeviceN polygon
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.3;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        lines += 1;
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createTarget(mako, box, lines, angle), deviceNBrush));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createTarget(mako, box, lines + 4, angle), deviceNBrush));

        // Draw All color polygon
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.3;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        lines += 1;
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createTarget(mako, box, lines, angle), allBrush));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createTarget(mako, box, lines + 4, angle), allBrush));

        IPDFOutput::create(mako)->writeAssembly(assembly, "test.pdf");
    }
    catch (IError& e)
    {
        const String errorFormatString = getEDLErrorString(e.getErrorCode());
        std::wcerr << L"Exception thrown: " << e.getErrorDescription(errorFormatString) << std::endl;
        return static_cast<int>(e.getErrorCode());
    }
    catch (std::exception& e)
    {
        std::wcerr << L"std::exception thrown: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

IDOMPathGeometryPtr createHexagon(const IJawsMakoPtr& mako, FRect box, double rotation)
{
    return IDOMPathGeometry::createPolygon(mako, box, 6, rotation);
}

IDOMPathGeometryPtr createTarget(const IJawsMakoPtr& mako, FRect box, double nLines, double rotation)
{
    double angle = rotation * (PI / 180.0);
    const double incr = 2.0 * PI / nLines;

    const auto radiusX = box.dX / 2.0;
    const auto radiusY = box.dY / 2.0;
    const auto centerX = box.x + radiusX;
    const auto centerY = box.y + radiusY;

    const auto geometryBuilder = IDOMPathGeometryBuilder::create(mako);
    const auto center = FPoint(centerX, centerY);
    geometryBuilder->moveTo(center);

    double endX = 0.0, endY = 0.0;
    for (int i = 0; i < nLines; i++)
    {
        endX = centerX + radiusX * cos(angle);
        endY = centerY + radiusY * sin(angle);
        geometryBuilder->lineTo(FPoint(endX, endY));
        geometryBuilder->moveTo(center);
        angle += incr;
    }
    geometryBuilder->close();
    return geometryBuilder->createGeometry(mako, IDOMPathGeometry::eFRNonZero);
}

IDOMColorPtr MakeDeviceNColor(const IJawsMakoPtr& mako, const U8String& name, const CDoubleVect& representation, IDOMColorSpacePtr alternateSpace)
{
    // Create a vector of colorants with one entry
    auto colorants = IDOMColorSpaceDeviceN::CColorantInfoVect();
    const auto colorantInfo = IDOMColorSpaceDeviceN::CColorantInfo(name, representation);
    colorants.append(colorantInfo);

    // Create the DeviceN space
    const auto colorSpace = IDOMColorSpaceDeviceN::create(mako, colorants, alternateSpace);

    // Return the new spot color
    return IDOMColor::create(mako, colorSpace, 1.0, 1.0);
}