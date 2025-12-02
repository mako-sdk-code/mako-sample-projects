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

public class TextTransformImplementation : ICustomTransform.IImplementation
{
    private readonly IJawsMako m_mako;
    private readonly float m_textInkValue;

    public TextTransformImplementation(IJawsMako mako, float textInkValue)
    {
        m_mako = mako;
        m_textInkValue = textInkValue;
    }

    public override IDOMNode transformGlyphs(ICustomTransform.IImplementation genericImplementation, IDOMGlyphs glyphs, ref bool changed, CTransformState state)
    {
        try
        {
            var fill = glyphs.getFill();
            if (fill.getBrushType() != IDOMBrush.eBrushType.eSolidColor)
                return genericImplementation.transformGlyphs(null, glyphs, ref changed, state);

            var colorBrush = IDOMSolidColorBrush.fromRCObject(fill);
            var color = colorBrush.getColor();

            if (color.getColorSpace().equals(IDOMColorSpaceDeviceCMYK.create(m_mako)) &&
                Math.Abs(color.getComponentValue(3) - 1.0f) < 0.0001f)
            {
                var newBrush = IDOMSolidColorBrush.createSolidCmyk(m_mako, 0.0f, 0.0f, 0.0f, m_textInkValue);
                glyphs.setFill(newBrush);
                changed = true;
                return glyphs;
            }
        }
        catch (MakoException e)
        {
            string errorFormatString = jawsmakoIF_csharp.getEDLErrorString(e.m_errorCode);
            Console.Error.WriteLine($"Exception thrown: {e.m_msg}");
        }
        catch (Exception e)
        {
            Console.Error.WriteLine($"std::exception thrown: {e.Message}");
        }

        return genericImplementation.transformGlyphs(null, glyphs, ref changed, state);
    }
}

