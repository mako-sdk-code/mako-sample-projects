/* ---------------------------------------------------------------------- -
 *  <copyright file="TextTransformImplementation.h" company="Global Graphics Software Ltd">
 *      Copyright (c) 2024 Global Graphics Software Ltd. All rights reserved.
 *  </copyright>
 *  <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Global Graphics Software Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 *  </summary>
 * -----------------------------------------------------------------------
 */
#pragma once

#include "jawsmako/customtransform.h"

using namespace JawsMako;
using namespace EDL;

class TextTransformImplementation : public ICustomTransform::IImplementation
{
public:
    TextTransformImplementation(const IJawsMakoPtr& jawsMako, const float& textInkValue);
    IDOMNodePtr transformGlyphs(IImplementation* genericImplementation, const IDOMGlyphsPtr& glyphs, bool& changed, const CTransformState& state) override;

private:
    IJawsMakoPtr m_mako;
    float m_textInkValue;
}; 
