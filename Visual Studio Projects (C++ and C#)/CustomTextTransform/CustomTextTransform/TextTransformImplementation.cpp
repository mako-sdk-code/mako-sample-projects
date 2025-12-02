/* ---------------------------------------------------------------------- -
 *  <copyright file="TextTransformImplementation.cpp" company="Global Graphics Software Ltd">
 *      Copyright (c) 2024 Global Graphics Software Ltd. All rights reserved.
 *  </copyright>
 *  <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Global Graphics Software Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 *  </summary>
 * -----------------------------------------------------------------------
 */

#include <iostream>
#include "TextTransformImplementation.h"

TextTransformImplementation::TextTransformImplementation(const IJawsMakoPtr& jawsMako, const float& textInkValue) :
    m_mako(jawsMako), m_textInkValue(textInkValue)
{
}

IDOMNodePtr TextTransformImplementation::transformGlyphs(IImplementation* genericImplementation, const IDOMGlyphsPtr& glyphs, bool& changed, const CTransformState& state)
{
    try
    {
        const auto fill = glyphs->getFill();
        if (fill->getBrushType() != IDOMBrush::eSolidColor)
            return genericImplementation->transformGlyphs(nullptr, glyphs, changed, state);

        if (edlobj2IDOMSolidColorBrush(fill)->getColor()->getColorSpace()->equals(IDOMColorSpaceDeviceCMYK::create(m_mako)) &&
            edlobj2IDOMSolidColorBrush(fill)->getColor()->getComponentValue(3) == 1.0f)
        {
            glyphs->setFill(IDOMSolidColorBrush::createSolidCmyk(m_mako, 0.0f, 0.0f, 0.0f, m_textInkValue));
            changed = true;
            return glyphs;
        }
    }
    catch (IError& e)
    {
        const String errorFormatString = getEDLErrorString(e.getErrorCode());
        std::wcerr << L"Exception thrown: " << e.getErrorDescription(errorFormatString) << std::endl;
    }
    catch (std::exception& e)
    {
        std::wcerr << L"std::exception thrown: " << e.what() << std::endl;
    }
    return genericImplementation->transformGlyphs(nullptr, glyphs, changed, state);
}