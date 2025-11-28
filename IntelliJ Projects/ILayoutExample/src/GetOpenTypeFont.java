/* -----------------------------------------------------------------------
 * <copyright file="GetOpenTypeFont.java" company="Hybrid Software Helix Ltd">
 *  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

import com.globalgraphics.JawsMako.jawsmakoIF.IDOMFontOpenType;
import com.globalgraphics.JawsMako.jawsmakoIF.IJawsMako;

import java.util.ArrayList;

public class GetOpenTypeFont {
    private IJawsMako mako;
    public IDOMFontOpenType font;
    public long index;

    public GetOpenTypeFont(IJawsMako mako, ArrayList<String> fontsToTry) {
        long[] fontIndex = new long[1]; // In case the font is inside a TrueType collection
        for (String fontToTry : fontsToTry) {
            try {
                font = IDOMFontOpenType.fromRCObject(mako.findFont(fontToTry, fontIndex));
                break;
            } catch (Exception e) {
                // Bad or missing font - default to Arial
                font = IDOMFontOpenType.fromRCObject(mako.findFont("Arial", fontIndex));
            }
        }
        index = fontIndex[0];
    }
}
