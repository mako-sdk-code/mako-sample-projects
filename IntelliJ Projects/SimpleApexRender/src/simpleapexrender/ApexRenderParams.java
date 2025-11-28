/*
 * Copyright (C) 2024-2025 Global Graphics Software Ltd. All rights reserved.
 *
 */

package simpleapexrender;

import java.util.*;
import com.globalgraphics.JawsMako.jawsmakoIF.*;

public class ApexRenderParams
{
    public IJawsMako                m_jawsMako;
    public IEDLClassFactory         m_factory;

    public String                   m_progName;

    public String                   m_inputFilePath;
    public eFileFormat              m_inputFileFormat;

    public String                   m_outputFilePath; // This may be a printf-style format string
    public eRenderedFormat          m_renderedFormat;
    public boolean                  m_bigTIFF; // If using tiff, whether or not to use bigTIFF

    public int                      m_xResolution;
    public int                      m_yResolution;
    public byte                     m_depth;
    public IDOMColorSpace           m_processSpace;
    public boolean                  m_useOutputIntent;
    public IDOMColorSpace           m_finalSpace;
    public boolean                  m_overprintPreview;
    public byte                     m_aaFactor;
    public boolean                  m_alpha;
    public boolean                  m_use16BitInternalRendering;
    public boolean                  m_ignoreIntercepts;
    
    public eOptionalContentEvent    m_optionalContentEvent;

    public int                      m_threadsPerGpu;
    public boolean                  m_useAllGpus;
    public boolean                  m_excludeIntegratedGpus;

    private ArrayDeque<String>      m_commandLineParams;

    public ApexRenderParams(IJawsMako jawsMako, String progName)
    {
        m_jawsMako          = jawsMako;
        m_progName          = progName;
        m_commandLineParams = new ArrayDeque<>();
    }
    
    public void init()
    {
        m_factory                   = m_jawsMako.getFactory();

        // Defaults
        m_xResolution               = 150;
        m_yResolution               = 150;
        m_depth                     = 8;
        m_bigTIFF                   = false;
        m_processSpace              = IDOMColorSpaceDeviceRGB.create(m_factory);
        m_useOutputIntent           = false;
        m_overprintPreview          = false;
        m_aaFactor                  = 1;
        m_alpha                     = false;
        m_use16BitInternalRendering = false;
        m_optionalContentEvent      = eOptionalContentEvent.eOCEPrint;
        m_useAllGpus                = false;
        m_excludeIntegratedGpus     = true;
        m_threadsPerGpu             = 1;
        m_ignoreIntercepts          = false;
    }

    // Add a command line parameter
    public void addCommandLineParameter(String paramStr)
    {
        m_commandLineParams.add(paramStr);
    }

    // Once we've got all parameters the need to be
    // grouped and interpreted to find out which ones are
    //  - input filename and file format
    //  - output filename and file format
    //  - list of render parameters
    public boolean parseCommandLineParameters() throws Exception
    {
        // There should be at least two parameters - input and output
        if (m_commandLineParams.size() < 2)
        {
            usage("Needs at least an input and an output file.");
            return false;
        }

        // Input path and format
        m_inputFilePath   = m_commandLineParams.pop();
        m_inputFileFormat = formatFromPath(m_inputFilePath);

        // Output path and format
        m_outputFilePath = m_commandLineParams.pop();

        // Might be none
        if (m_outputFilePath.equals("none"))
        {
            m_renderedFormat = eRenderedFormat.eRFNone;
        }
        else
        {
            m_renderedFormat = renderFormatFromPath(m_outputFilePath);
        }

        init();

        // Any others will be just '=' separated values.
        while (!m_commandLineParams.isEmpty())
        {
            String paramStr = m_commandLineParams.pop().toLowerCase();

            // Find the equals
            int equalsPos = paramStr.indexOf("=");
            if (equalsPos < 0)
            {
                String msg = String.format("Paramater <%s> is missing a parameter value.", paramStr);
                usage(msg);
                return false;
            }

            String key   = paramStr.substring(0, equalsPos);
            String value = paramStr.substring(equalsPos + 1);

            if (!setParameter(key, value))
            {
                String msg = String.format("Error while setting parameter <%s>.", key);
                usage(msg);
                return false;
            }
        }

        return true;
    }

    // Print usage
    public void usage(String errMsg)
    {
        System.out.printf("\n");
        System.out.printf("%s\n", errMsg);
        System.out.printf("\n");
        
        System.out.printf("Usage: %s <source file> <output file or file pattern> [ key=value ... ]\n", m_progName);
        System.out.printf("Where: \n");
        System.out.printf(" <source file>                       Path to any input file supported by Mako\n");
        System.out.printf(" <output file | file pattern | none> Can either be a PDF file, 'none', or a printf style pattern\n");
        System.out.printf("                                     for a PNG, JPEG or TIFF where %%d is replaced with the page\n");
        System.out.printf("                                     number (starting from 1).\n");
        System.out.printf("                                     For example:\n");
        System.out.printf("                                       - \"out.pdf\", where each page will be rendered\n");
        System.out.printf("                                         into images within a single pdf \"out.pdf\".\n");
        System.out.printf("                                       - \"out%%d.png\", where each page will be rendered\n");
        System.out.printf("                                         into a series of PNG files (out1.png, out2.png etc.\n");
        System.out.printf("                                       - \"out%%d.tif\", where each page will be rendered\n");
        System.out.printf("                                         into a series of TIFF files (out1.tif, out2.tif etc.\n");
        System.out.printf("                                       - \"out%%d.jpg\", where each page will be rendered\n");
        System.out.printf("                                         into a series of JPEG files (out1.jpg, out2.jpg etc.\n");
        System.out.printf("                                     Here, PNG only supports RGB formats.\n");
        System.out.printf("                                     Specifying 'none' as the file name results in rendering only with no output.\n");
        System.out.printf(" [ key=value ... ]                   A trailing series of key/value pairs can be provided,\n");
        System.out.printf("                                     Supported options are:\n");
        System.out.printf("                                       - res=<integral resolution>\n");
        System.out.printf("                                       - xres=<integral resolution for x-axis>\n");
        System.out.printf("                                       - yres=<integral resolution for y-axis>\n");
        System.out.printf("                                       - alpha=<true | false>\n");
        System.out.printf("                                       - depth=<8| 16>\n");
        System.out.printf("                                       - overprintpreview=<true | false> (cannot be used with alpha)\n");
        System.out.printf("                                       - aafactor=<1 | 2 | 3 | 4>\n");
        System.out.printf("                                       - bigtiff=<true | false>\n");
        System.out.printf("                                       - use16BitInternalRendering=<true | false>\n");
        System.out.printf("                                         If true, then all intermediate rendering is performed using 16-bit\n");
        System.out.printf("                                         depth before being downconverted to 8-bit before being copied from\n");
        System.out.printf("                                         the GPU. The default is false.\n");
        System.out.printf("                                       - processspace=<gray | rgb | cmyk>\n");
        System.out.printf("                                       - processprofile=<path to a gray, RGB or CMYK ICC profile>\n");
        System.out.printf("                                       - useoutputintent=<true | false>\n");
        System.out.printf("                                         If true, and the input has a PDF CMYK output intent, then the output\n");
        System.out.printf("                                         intent will be used as the process space, and if the final space is\n");
        System.out.printf("                                         CMYK, then this intent will be use as the final space.\n");
        System.out.printf("                                       - finalspace=<gray | rgb | cmyk>\n");
        System.out.printf("                                       - finalprofile=<path to a gray, RGB or CMYK ICC profile>\n");
        System.out.printf("                                       - optionalcontentevent=<view | print | export> The default is print.\n");
        System.out.printf("                                       - threadspergpu=<integer> The number of tile threads to use per GPU. The default is 1.\n");
        System.out.printf("                                       - useallgpus=<true | false> If true, multiple renderers will be used for\n");
        System.out.printf("                                         different pages if multiple GPUS are present. The default is false.\n");
        System.out.printf("                                       - excludeintegratedgpus=<true | false> If true, integrated GPUs will not be used for\n");
        System.out.printf("                                         usallgpus. The default is true.\n");
    }


    // Determine the associated format for a given path from the file extension
    eFileFormat formatFromPath(String path) throws Exception
    {
        // Get the extension in lower case
        int idx = path.lastIndexOf('.');
        if (idx < 0)
        {
            // Cannot determine the extension if there isn't one!
            String message = String.format("Cannot determine file extension for path %s", path);
            throw new Exception(message);
        }

        String extension = path.substring(idx).toLowerCase();

        HashMap<String, eFileFormat> extMap = new HashMap<>();

        extMap.put (".xps",   eFileFormat.eFFXPS    );
        extMap.put (".pdf",   eFileFormat.eFFPDF    );
        extMap.put (".svg",   eFileFormat.eFFSVG    );
        extMap.put (".ps",    eFileFormat.eFFPS     );
        extMap.put (".eps",   eFileFormat.eFFEPS    );
        extMap.put (".pcl",   eFileFormat.eFFPCL5   );
        extMap.put (".pxl",   eFileFormat.eFFPCLXL  );
        extMap.put (".ijp",   eFileFormat.eFFIJPDS  );
        extMap.put (".zip",   eFileFormat.eFFPPML   );
        extMap.put (".oxps",  eFileFormat.eFFOXPS   );
        extMap.put (".prn",   eFileFormat.eFFPRN    );

        if (!extMap.containsKey(extension))
        {
            String message = String.format("Unsupported file type for path %s", path);
            throw new Exception(message);
        }

        return extMap.get (extension);
    }

    // And for the output
    eRenderedFormat renderFormatFromPath(String path) throws Exception
    {
        // Get the extension in lower case
        // Get the extension in lower case
        int idx = path.lastIndexOf('.');
        if (idx < 0)
        {
            // Cannot determine the extension if there isn't one!
            String message = String.format("Cannot determine file extension for path %s", path);
            throw new Exception(message);
        }

        String extension = path.substring(idx).toLowerCase();

        HashMap<String, eRenderedFormat> extMap = new HashMap<>();
        extMap.put (".pdf",   eRenderedFormat.eRFPDF  );
        extMap.put (".tif",   eRenderedFormat.eRFTIFF );
        extMap.put (".png",   eRenderedFormat.eRFPNG  );
        extMap.put (".jpg",   eRenderedFormat.eRFJPEG );

        if (!extMap.containsKey(extension))
        {
            String message = String.format("Unsupported render type for path %s", path);
            throw new Exception(message);
        }

        return extMap.get (extension);
    }

    private static class ParseRet<T>
    {
        public final boolean    ok;
        public final T          val;
        
        public ParseRet (boolean ok, T val)
        {
            this.ok = ok;
            this.val = val;
        }
    }

    private ParseRet<Boolean> parse (String str, boolean defaultVal)
    {
        str = str.toLowerCase();

        boolean val = defaultVal;
        boolean ok  = true;
        
        switch (str)
        {
            case "true" : 
            case "1" :
                val = true;
                break;
                
            case "false" :
            case "0" :
                val = false;
                break;

            default:
                ok = false;
        }

        return new ParseRet (ok, val);
    }
    
    private ParseRet<Integer> parse (String str, int defaultVal)
    {
        str = str.toLowerCase();

        int             val = defaultVal;
        boolean         ok  = true;
        
        try
        {
            val = Integer.parseInt(str);
        }
        catch (NumberFormatException ex)
        {
            ok = false;
        }
        
        return new ParseRet (ok, val);
    }
        

    private ParseRet<Byte> parse (String str, byte defaultVal)
    {
        str = str.toLowerCase();

        int             val = defaultVal;
        boolean         ok  = true;
        
        try
        {
            val = Byte.parseByte(str);
        }
        catch (NumberFormatException ex)
        {
            ok = false;
        }
        
        return new ParseRet (ok, val);
    }
        
   
    private ParseRet<IDOMColorSpace> parse (String str, IDOMColorSpace defaultVal)
    {
        str = str.toLowerCase();

        IDOMColorSpace  val = defaultVal;
        boolean         ok = true;
        
        switch (str)
        {
            case "gray":
                val = IDOMColorSpaceDeviceGray.create(m_factory);
                break;

            case "rgb":
                val = IDOMColorSpaceDeviceRGB.create(m_factory);
                break;

            case "cmyk":
                val = IDOMColorSpaceDeviceCMYK.create(m_factory);
                break;
                
            default:
                ok = false;
        }
        
        return new ParseRet (ok, val);
    }

    // Set commandline parameters for the app.
    // Return true if the option is supported and false otherwise.
    boolean setParameter(String key, String value)
    {
        try
        {
            switch (key)
            {
                case "res":
                    {
                        var ret = parse (value, m_xResolution);
                        if (! ret.ok)
                            return false;
                        m_xResolution = ret.val;
                    }
                    if (m_xResolution == 0)
                        return false;
                    m_yResolution = m_xResolution;
                    break;

                case "xres":
                    {
                        var ret = parse (value, m_xResolution);
                        if (! ret.ok)
                            return false;
                        m_xResolution = ret.val;
                    }
                    if (m_xResolution == 0)
                        return false;
                    break;
                    
                case "yres":
                    {
                        var ret = parse (value, m_yResolution);
                        if (! ret.ok)
                            return false;
                        m_yResolution = ret.val;
                    }
                    if (m_yResolution == 0)
                        return false;
                    break;

                case "aafactor":
                    {
                        var ret = parse (value, m_aaFactor);
                        if (! ret.ok)
                            return false;
                        m_aaFactor = ret.val;
                    }
                    if (m_aaFactor == 0)
                        return false;
                    break;

                case "alpha":
                    {
                        var ret = parse (value, m_alpha);
                        if (! ret.ok)
                            return false;
                        m_alpha = ret.val;
                    }
                    break;
                    
                case "depth":
                    {
                        var ret = parse (value, m_depth);
                        if (! ret.ok)
                            return false;
                        m_depth  = ret.val;
                    }
                    if (m_depth != 8 && m_depth != 16)
                        return false;
                    break;

                case "use16bitinternalbitrendering":
                    {
                        var ret = parse (value, m_use16BitInternalRendering);
                        if (! ret.ok)
                            return false;
                        m_use16BitInternalRendering = ret.val;
                    }
                    break;

                case "overprintpreview":
                    {
                        var ret = parse (value, m_overprintPreview);
                        if (! ret.ok)
                            return false;
                        m_overprintPreview = ret.val;
                    }
                    break;

                case "bigtiff":
                    {
                        var ret = parse (value, m_bigTIFF);
                        if (! ret.ok)
                            return false;
                        m_bigTIFF = ret.val;
                    }
                    break;

                case "processspace":
                    {
                        var ret = parse (value, m_processSpace);
                        if (! ret.ok)
                            return false;
                        m_processSpace = ret.val;
                    }
                    break;

                case "processprofile":
                    {
                        IDOMICCProfile profile = IDOMICCProfile.create(m_factory, IInputStream.createFromFile(m_factory, value));
                        m_processSpace = IDOMColorSpaceICCBased.create(m_factory, profile);
                    }
                    break;
                    
                case "useoutputintent":
                    {
                        var ret = parse (value, m_useOutputIntent);
                        if (! ret.ok)
                            return false;
                        m_useOutputIntent = ret.val;
                    }
                    break;
                    
                case "finalspace":
                    {
                        var ret = parse (value, m_finalSpace);
                        if (! ret.ok)
                            return false;
                        m_finalSpace = ret.val;
                    }
                    break;

                case "finalprofile":
                    {
                        IDOMICCProfile profile = IDOMICCProfile.create(m_factory, IInputStream.createFromFile(m_factory, value));
                        m_finalSpace = IDOMColorSpaceICCBased.create(m_factory, profile);
                    }
                    break;
                    
                case "optionalcontentevent":

                    switch (value)
                    {
                        case "view":
                            m_optionalContentEvent = eOptionalContentEvent.eOCEView;
                            break;
                            
                        case "print":
                            m_optionalContentEvent = eOptionalContentEvent.eOCEPrint;
                            break;
                            
                        case "export":
                            m_optionalContentEvent = eOptionalContentEvent.eOCEExport;
                            break;
                    
                        default:
                            return false;
                    }
                    break;
                    
                case "threadspergpu":
                    {
                        var ret = parse (value, m_threadsPerGpu);
                        if (! ret.ok)
                            return false;
                        m_threadsPerGpu = ret.val;
                    }
                    if (m_threadsPerGpu == 0)
                        return false;
                    break;

                case "useallgpus":
                    {
                        var ret = parse (value, m_useAllGpus);
                        if (! ret.ok)
                            return false;
                        m_useAllGpus = ret.val;
                    }
                    break;

                case "excludeintegratedgpus":
                    {
                        var ret = parse (value, m_excludeIntegratedGpus);
                        if (! ret.ok)
                            return false;
                        m_excludeIntegratedGpus = ret.val;
                    }
                    break;
                    
                default:
                    // Unknown parameter
                    return false;
            }
        }
        catch (Exception ex)
        {
            return false;
        }

        return true;
    }
}
