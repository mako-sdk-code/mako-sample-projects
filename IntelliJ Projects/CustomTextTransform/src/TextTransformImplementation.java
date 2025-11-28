/* -----------------------------------------------------------------------
 * <copyright file="TextTransformImplementation.java" company="Hybrid Software Helix Ltd">
 *  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  Custom transform implementation to adjust black text ink values.
 * </summary>
 * -----------------------------------------------------------------------
 */

import com.globalgraphics.JawsMako.jawsmakoIF.*;

public class TextTransformImplementation extends ICustomTransform.IImplementation {
    private final IJawsMako m_mako;
    private final float m_textInkValue;

    public TextTransformImplementation(IJawsMako mako, float textInkValue)
    {
        this.m_mako = mako;
        this.m_textInkValue = textInkValue;
    }

    @Override
    public IDOMNode transformGlyphs(ICustomTransform.IImplementation genericImplementation,
                                    IDOMGlyphs glyphs,
                                    boolean[] changed,
                                    CTransformState state)
    {
        try
        {
            IDOMBrush fill = glyphs.getFill();
            if (fill.getBrushType() != IDOMBrush.eBrushType.eSolidColor)
                return genericImplementation.transformGlyphs(null, glyphs, changed, state);

            IDOMSolidColorBrush colorBrush = IDOMSolidColorBrush.fromRCObject(fill);
            IDOMColor color = colorBrush.getColor();

            if (color.getColorSpace().equals(IDOMColorSpaceDeviceCMYK.create(m_mako.getFactory())) &&
                    Math.abs(color.getComponentValue(3) - 1.0f) < 0.0001f)
            {
                IDOMSolidColorBrush newBrush = IDOMSolidColorBrush.createSolidCmyk(
                        m_mako.getFactory(), 0.0f, 0.0f, 0.0f, m_textInkValue
                );
                glyphs.setFill(newBrush);
                changed[0] = true;
                return glyphs;
            }
        }
        catch (Exception e)
        {
            System.err.printf("std::exception thrown: %s%n", e.getMessage());
        }

        return genericImplementation.transformGlyphs(null, glyphs, changed, state);
    }
}
