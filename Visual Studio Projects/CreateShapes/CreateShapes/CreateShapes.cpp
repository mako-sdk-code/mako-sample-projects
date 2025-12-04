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
IDOMColorPtr MakeSeparationColor(const IJawsMakoPtr& mako, const U8String& name, const CDoubleVect& representation, IDOMColorSpacePtr alternateSpace = IDOMColorSpacePtr());

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

        // Create an 'All' spot color space and a brush to draw with
        const auto spotColorAll = MakeSeparationColor(mako, "All", CDoubleVect{ 1.0, 1.0, 1.0, 1.0 });
        const auto all = IDOMSolidColorBrush::create(mako, spotColorAll);

        // Create a 'Rot' spot color
        const auto spotColorRot = MakeSeparationColor(mako, "Rot", CDoubleVect{ 0.0, 1.0, 1.0, 0.0 });
        const auto rot = IDOMSolidColorBrush::create(mako, spotColorRot);

        // Create a 'Blau' spot color
        const auto spotColorBlau = MakeSeparationColor(mako, "Blau", CDoubleVect{ 1.0, 1.0, 0.0, 0.0 });
        const auto blau = IDOMSolidColorBrush::create(mako, spotColorBlau);

        // Create a 'Grun' spot color
        const auto spotColorGrun = MakeSeparationColor(mako, "Grun", CDoubleVect{ 1.0, 0.0, 1.0, 0.0 });
        const auto grun = IDOMSolidColorBrush::create(mako, spotColorGrun);

        // Create an LAB spot color
        const auto spotColorBlack = MakeSeparationColor(mako, "LABBlack", CDoubleVect{ 0.0, 0.0, 0.0 });
        const auto labBlack = IDOMSolidColorBrush::create(mako, spotColorBlack);

        // Draw some boxes
        const auto masterBoxSize = FPoint(80, 80);
        auto boxSize = masterBoxSize;
        auto origin = FPoint(80, 80);
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

        // Draw Red box
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::create(mako, box), rot));
        start.x += boxSize.x;

        // Draw Green box
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::create(mako, box), grun));
        start.x += boxSize.x;

        // Draw Blue box
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::create(mako, box), blau));
        start.x += boxSize.x;

        // Draw Black box
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::create(mako, box), labBlack));
        
        // Draw a border in All
        const uint32 strokeWidth = 20;
        box = FRect(origin.x - strokeWidth / 2.0, origin.y - strokeWidth / 2.0, (boxSize.x * 8) + strokeWidth, boxSize.y + strokeWidth);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, IDOMPathGeometry::create(mako, box), all, FMatrix(),
            IDOMPathGeometryPtr(), strokeWidth));

        // Draw some other shapes below
        boxSize = masterBoxSize;
        origin.y += 160;
        start = origin;

        // Draw Cyan circle
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, IDOMPathGeometry::createEllipse(mako, box), cyan));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::createEllipse(mako, box), cyan));

        // Draw Magenta circle
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.4;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, IDOMPathGeometry::createEllipse(mako, box), magenta));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::createEllipse(mako, box), magenta));

        // Draw Yellow circle
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.4;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, IDOMPathGeometry::createEllipse(mako, box), yellow));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::createEllipse(mako, box), yellow));

        // Draw Black circle
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.4;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, IDOMPathGeometry::createEllipse(mako, box), black));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::createEllipse(mako, box), black));

        // Draw some hexagons
        boxSize = masterBoxSize;
        origin.y += 200;
        start = origin;

        // Draw Red hexagon
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createHexagon(mako, box), rot));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, createHexagon(mako, box), rot));

        // Draw Green hexagon
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.4;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createHexagon(mako, box), grun));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, createHexagon(mako, box), grun));

        // Draw Blue hexagon
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.4;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createHexagon(mako, box), blau));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, createHexagon(mako, box), blau));

        // Draw "labBlack" color hexagon
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.4;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createHexagon(mako, box), labBlack));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, createHexagon(mako, box), labBlack));

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
        boxSize.x *= 1.4;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, IDOMPathGeometry::createPolygon(mako, box, sides, angle), magenta));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::createPolygon(mako, box, sides, angle), magenta));

        // Draw Yellow polygon
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.4;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, IDOMPathGeometry::createPolygon(mako, box, sides, angle), yellow));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::createPolygon(mako, box, sides, angle), yellow));

        // Draw Black polygon
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.4;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, IDOMPathGeometry::createPolygon(mako, box, sides, angle), black));
        box.y += boxSize.y * 1.1;
        fixedPage->appendChild(IDOMPathNode::createFilled(mako, IDOMPathGeometry::createPolygon(mako, box, sides, angle), black));

        // ----- Draw some targets -----
        
        auto lines = 9;
        angle = 0.0;
        boxSize = masterBoxSize;
        origin.y += 200;
        start = origin;

        // Draw Red polygon
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createTarget(mako, box, lines, angle), rot));
        box.y += boxSize.y * 1.1;
        lines = 8;
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createTarget(mako, box, lines, angle), rot));

        // Draw Green polygon
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.4;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        lines = 4;
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createTarget(mako, box, lines, angle), grun));
        box.y += boxSize.y * 1.1;
        lines = 12;
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createTarget(mako, box, lines, angle), grun));

        // Draw Blue polygon
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.4;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        lines = 4;
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createTarget(mako, box, lines, angle), blau));
        box.y += boxSize.y * 1.1;
        lines = 8;
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createTarget(mako, box, lines, angle), blau));

        // Draw "labBlack" color polygon
        start.x += boxSize.x * 1.2;
        boxSize.x *= 1.4;
        box = FRect(start.x, start.y, boxSize.x, boxSize.y);
        lines = 3;
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createTarget(mako, box, lines, angle), labBlack));
        box.y += boxSize.y * 1.1;
        lines = 9;
        fixedPage->appendChild(IDOMPathNode::createStroked(mako, createTarget(mako, box, lines, angle), labBlack));

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

// Create a new DeviceN color space with the given name and one colorant
IDOMColorPtr MakeSeparationColor(const IJawsMakoPtr& mako, const U8String& name, const CDoubleVect& representation, IDOMColorSpacePtr alternateSpace)
{
    // Create a vector of colorants with one entry
    auto colorants = IDOMColorSpaceDeviceN::CColorantInfoVect();

    if (representation.size() == 3)
    {
        const auto colorantInfo = IDOMColorSpaceDeviceN::CColorantInfo(
            name, { representation[0], representation[1], representation[2] }
        );
        colorants.append(colorantInfo);

        if (!alternateSpace)
            alternateSpace = IDOMColorSpaceLAB::create(mako, 0.9642f, 1.0000f, 0.8249f, 0.0f, 0.0f, 0.0f, -128.0f, 127.0f,
                -128.0f, 127.0f);
    }
    else if (representation.size() == 4)
    {
        const auto colorantInfo = IDOMColorSpaceDeviceN::CColorantInfo(
            name, { representation[0], representation[1], representation[2], representation[3]}
        );
        colorants.append(colorantInfo);

        if (!alternateSpace)
            alternateSpace = IDOMColorSpaceDeviceCMYK::create(mako);
    }

    if (colorants.empty())
        return IDOMColorPtr();

    // Create the DeviceN space
    const auto colorSpace = IDOMColorSpaceDeviceN::create(mako, colorants, alternateSpace);

    // Return the new spot color
    return IDOMColor::create(mako, colorSpace, 1.0, 1.0);
}