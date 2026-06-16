/* -----------------------------------------------------------------------
     * <copyright file="TextTransformImplementation.cs" company="Hybrid Software Helix Ltd">
     *  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
     * </copyright>
     * <summary>
     *  Custom transform implementation to adjust black text ink values.
     * </summary>
     * -----------------------------------------------------------------------
     */

using JawsMako;

namespace CustomTextTransformCS;

public class TextTransformImplementation(IJawsMako mako, float textInkValue) : ICustomTransform.IImplementation
{
    public override IDOMNode transformGlyphs(ICustomTransform.IImplementation genericImplementation, IDOMGlyphs glyphs, ref bool changed, CTransformState state)
    {
        try
        {
            using var fill = glyphs.getFill();
            if (fill.getBrushType() != IDOMBrush.eBrushType.eSolidColor)
                return genericImplementation.transformGlyphs(null, glyphs, ref changed, state);

            using var colorBrush = IDOMSolidColorBrush.fromRCObject(fill);
            using var color = colorBrush.getColor();

            if (color.getColorSpace().equals(IDOMColorSpaceDeviceCMYK.create(mako)) &&
                Math.Abs(color.getComponentValue(3) - 1.0f) < 0.0001f)
            {
                using var newBrush = IDOMSolidColorBrush.createSolidCmyk(mako, 0.0f, 0.0f, 0.0f, textInkValue);
                glyphs.setFill(newBrush);
                changed = true;
                return glyphs;
            }
        }
        catch (MakoException e)
        {
            Console.Error.WriteLine($"Exception thrown: {e.m_msg}");
        }
        catch (Exception e)
        {
            Console.Error.WriteLine($"std::exception thrown: {e.Message}");
        }

        return genericImplementation.transformGlyphs(null, glyphs, ref changed, state);
    }
}