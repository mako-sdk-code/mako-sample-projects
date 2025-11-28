/*
 * Copyright (C) 2025 Global Graphics Software Ltd. All rights reserved.
 */

package simpleapexrender;

import java.util.*;
import com.globalgraphics.JawsMako.jawsmakoIF.*;
import com.globalgraphics.JawsMako.jawsmakoIF.jawsmakoIF.*;

public class FindSpots
{
    // Utility to find all spot colorants on a page
    public static void findAllSpots (IJawsMako jawsMako,
                                    IDOMFixedPage content,
                                    IDOMColorSpace outputSpace,
                                    CEDLVectColorantInfo spots)
    {
        // Scan for inks
        CEDLVectCInkInfo inks = IRendererTransform.findInks (jawsMako, content);

        // Convert to DeviceN colourants
        CEDLVectColorantInfo candidates = IRendererTransform.inkInfoToColorantInfo (jawsMako, inks, outputSpace);

        // Scan and strip any process names or /All or none
        
        List<String> toIgnore = new ArrayList<> ();
        toIgnore.add("Magenta");
        toIgnore.add("Yellow");
        toIgnore.add("Black");
        toIgnore.add("All");
        toIgnore.add("None");

        spots.resize(0);
        for (IDOMColorSpaceDeviceN.CColorantInfo info : candidates.toArray ())
        {
            if (! toIgnore.contains(info.getName()))
                spots.append (info);
        }

    }
}