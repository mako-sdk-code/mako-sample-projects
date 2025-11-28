/*
 * Copyright (C) 2024-2025 Global Graphics Software Ltd. All rights reserved.
 */
 
package simpleapexrender;

// Our example can render to either:
// - Images embedded in PDF
// - Images (TIFF, PNG, JPEG)
// - Nothing - render and discard
public enum eRenderedFormat
{
    eRFPDF,
    eRFTIFF,
    eRFPNG,
    eRFJPEG,
    eRFNone
};
