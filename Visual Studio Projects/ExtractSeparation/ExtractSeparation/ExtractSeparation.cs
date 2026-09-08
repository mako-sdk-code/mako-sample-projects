/* --------------------------------------------------------------------------------
 *  <copyright file="ExtractSeparation.cs" company="Hybrid Software Helix Ltd">
 *    Copyright (c) 2026 Hybrid Software Helix Ltd. All rights reserved.
 *  </copyright>
 *  <summary>
 *    This example is provided on an "as is" basis and without warranty of any kind.
 *    Hybrid Software Helix Ltd. does not warrant or make any representations
 *    regarding the use or results of use of this example.
 *  </summary>
 * ---------------------------------------------------------------------------------
 */

using JawsMako;
using static JawsMako.IRendererTransform;

namespace ExtractSeparation;

internal static class Program
{
    private static int Main(string[] args)
    {
        if (args.Length != 3)
        {
            Console.WriteLine("Usage: ExtractSeparation <input.pdf> <output.pdf> <separation-name>");
            return 1;
        }

        var inputPath = Path.GetFullPath(args[0]);
        var outputPath = Path.GetFullPath(args[1]);
        var separationName = args[2];

        if (!File.Exists(inputPath))
        {
            Console.Error.WriteLine($"Input file not found: {inputPath}");
            return 1;
        }

        try
        {
            using var jawsMako = IJawsMako.create();
            IJawsMako.enableAllFeatures(jawsMako);

            if (!SaveSeparation(jawsMako, inputPath, outputPath, separationName))
            {
                Console.Error.WriteLine($"The separation '{separationName}' was not found on the first page.");
                return 2;
            }

            Console.WriteLine($"Created: {outputPath}");
            return 0;
        }
        catch (Exception exception)
        {
            Console.Error.WriteLine(exception.Message);
            return 3;
        }
    }

    /// <summary>
    /// Extracts one separation from the first page of a PDF and writes it as a named
    /// DeviceN spot colour in a new, single-page PDF.
    /// </summary>
    private static bool SaveSeparation(
        IJawsMako jawsMako,
        string inputPath,
        string outputPath,
        string separationName)
    {
        using var factory = jawsMako.getFactory();
        using var pdfInput = IPDFInput.create(jawsMako);
        using var inputAssembly = pdfInput.open(inputPath);
        using var inputDocument = inputAssembly.getDocument();

        if (inputDocument.getNumPages() == 0)
            return false;

        using var inputPage = inputDocument.getPage(0);
        using var sourceContent = inputPage.getContent();
        using var spotColorSpace = CreateSpotColorSpace(
            jawsMako,
            factory,
            sourceContent,
            separationName);

        if (spotColorSpace is null)
            return false;

        using var separator = ISeparator.create(jawsMako);
        separator.setEnableVectorMode(true);
        separator.separate(sourceContent);

        for (uint index = 0; index < separator.getNumSeparations(); index++)
        {
            using var separation = separator.getSeparation(index);

            if (!string.Equals(separation.second, separationName, StringComparison.OrdinalIgnoreCase))
                continue;

            using var separatedContent = separation.first;
            using var outputContent = CreateSpotMaskedContent(
                factory,
                separatedContent,
                spotColorSpace,
                sourceContent.getWidth(),
                sourceContent.getHeight());

            // Cloning retains the source page boxes and rotation. Annotations are
            // removed because they have not been filtered to the requested separation.
            using var outputPage = inputPage.clone();
            outputPage.removeAnnotations();
            outputPage.setContent(outputContent);

            using var outputAssembly = IDocumentAssembly.create(jawsMako);
            using var outputDocument = IDocument.create(jawsMako);
            outputAssembly.appendDocument(outputDocument);
            outputDocument.appendPage(outputPage);

            using var pdfOutput = IPDFOutput.create(jawsMako);
            pdfOutput.writeAssembly(outputAssembly, outputPath);
            return true;
        }

        return false;
    }

    /// <summary>
    /// Converts Mako's discovered ink information into a one-colourant DeviceN space.
    /// DeviceCMYK is selected explicitly as the alternate space, so zero represents
    /// paper and Mako can generate the appropriate tint transform automatically.
    /// </summary>
    private static IDOMColorSpaceDeviceN? CreateSpotColorSpace(
        IJawsMako jawsMako,
        IEDLClassFactory factory,
        IDOMFixedPage sourceContent,
        string separationName)
    {
        using var processSpace = IDOMColorSpaceDeviceCMYK.create(factory);
        using var inks = findInks(jawsMako, sourceContent);
        using var candidates = inkInfoToColorantInfo(jawsMako, inks, processSpace);
        using var selectedColorants = new CEDLVectColorantInfo();

        for (uint index = 0; index < candidates.size(); index++)
        {
            using var candidate = candidates.getitem(index);
            if (!string.Equals(candidate.name, separationName, StringComparison.OrdinalIgnoreCase))
                continue;

            selectedColorants.append(candidate);
            break;
        }

        if (selectedColorants.empty())
            return null;

        // This is a spot-only DeviceN space. Do not pass processSpace again as the
        // optional DeviceN process colour space: doing so writes a /Process dictionary,
        // and without a matching process-component list that dictionary is invalid.
        return IDOMColorSpaceDeviceN.create(factory, selectedColorants, processSpace);
    }

    /// <summary>
    /// Uses the greyscale plate produced by <see cref="ISeparator"/> as an inverted
    /// luminosity mask over a full-tint spot-colour rectangle.
    /// </summary>
    private static IDOMFixedPage CreateSpotMaskedContent(
        IEDLClassFactory factory,
        IDOMFixedPage separatedContent,
        IDOMColorSpaceDeviceN spotColorSpace,
        double pageWidth,
        double pageHeight)
    {
        var outputContent = IDOMFixedPage.create(factory, pageWidth, pageHeight);

        try
        {
            using var softGroup = IDOMTransparencyGroup.create(factory);
            using var graySpace = IDOMColorSpaceDeviceGray.create(factory);
            softGroup.setIsIsolated(true);
            softGroup.setColorSpace(graySpace);

            var child = separatedContent.getFirstChild();
            while (child is not null)
            {
                var currentChild = child;
                child = currentChild.getNextSibling();

                using (currentChild)
                {
                    currentChild.cloneTreeAndAppend(factory, softGroup);
                }
            }

            using var maskTransform = new FMatrix();
            using var maskBackdrop = IDOMColor.createSolidGray(factory, 1.0f);
            using var maskTransfer = CreateInvertingTransferFunction(factory);
            using var softMask = IDOMSoftMaskBrush.create(
                factory,
                softGroup,
                IDOMSoftMaskBrush.eSoftMaskType.eLuminosity,
                maskTransform,
                maskBackdrop,
                maskTransfer);

            using var spotBrush = IDOMSolidColorBrush.createWithSpaceAndComponentsFromArray(
                factory,
                spotColorSpace,
                1.0f,
                [1.0f]);
            using var pageBounds = new FRect(0.0, 0.0, pageWidth, pageHeight);
            using var geometry = IDOMPathGeometry.create(factory, pageBounds);
            using var path = IDOMPathNode.createFilled(factory, geometry, spotBrush);
            path.setOpacityMask(softMask);

            outputContent.appendChild(path);
            return outputContent;
        }
        catch
        {
            outputContent.Dispose();
            throw;
        }
    }

    /// <summary>
    /// Maps black to opaque and white to transparent because an ISeparator plate
    /// represents full ink as black and paper as white.
    /// </summary>
    private static IDOMFunction CreateInvertingTransferFunction(IEDLClassFactory factory)
    {
        using var parameters = new IDOMExponentialFunction.Data();
        parameters.domain.resize(2);
        parameters.domain[0] = 0.0f;
        parameters.domain[1] = 1.0f;
        parameters.range.resize(2);
        parameters.range[0] = 0.0f;
        parameters.range[1] = 1.0f;
        parameters.c0.resize(1);
        parameters.c0[0] = 1.0f;
        parameters.c1.resize(1);
        parameters.c1[0] = 0.0f;
        parameters.exponent = 1.0f;
        parameters.numOutputs = 1;

        var function = IDOMExponentialFunction.createInstance(factory);
        try
        {
            function.init(parameters);
            return function;
        }
        catch
        {
            function.Dispose();
            throw;
        }
    }
}
