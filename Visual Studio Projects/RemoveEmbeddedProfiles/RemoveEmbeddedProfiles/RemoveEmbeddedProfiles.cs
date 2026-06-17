/* --------------------------------------------------------------------------------
 *  <copyright file="RemoveEmbeddedProfiles.cs" company="Hybrid Software Helix Ltd">
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

namespace RemoveEmbeddedProfiles;

internal class RemoveEmbeddedProfiles
{
    private const string TestFilesPath = @"..\..\..\..\..\..\TestFiles\";

    static int Main(string[] args)
    {
        try
        {
            var defaultInputFile = Path.GetFullPath(
                Path.Combine(AppContext.BaseDirectory, TestFilesPath + "OpacityTest.pdf"));
            var inputFile = args.Length > 0 ? args[0] : defaultInputFile;
            var outputFile = args.Length > 1
                ? args[1]
                : Path.GetFileNameWithoutExtension(inputFile) + ".no-embedded-profiles.pdf";

            using var mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);

            using var pdfInput = IPDFInput.create(mako);
            using var assembly = pdfInput.open(inputFile);

            using var transform = ICustomTransform.create(mako, new RemoveEmbeddedProfilesTransform(mako));

            for (uint documentIndex = 0; documentIndex < assembly.getNumDocuments(); documentIndex++)
            {
                using var document = assembly.getDocument(documentIndex);

                for (uint pageIndex = 0; pageIndex < document.getNumPages(); pageIndex++)
                {
                    using var page = document.getPage(pageIndex);
                    transform.transformPage(page);
                }
            }
            
            IPDFOutput.create(mako).writeAssembly(assembly, IOutputStream.createToFile(mako, outputFile));
        }
        catch (MakoException e)
        {
            Console.WriteLine($"Exception thrown: {e.m_errorCode}: {e.m_msg}");
            return 1;
        }
        catch (Exception e)
        {
            Console.WriteLine($"Exception thrown: {e}");
            return 1;
        }

        return 0;
    }

    private static IDOMImage? SubstituteImageColorSpace(
        IEDLClassFactory factory,
        IDOMImage image)
    {
        using var replacementColorSpace = GetReplacementDeviceColorSpace(factory, image);
        if (replacementColorSpace == null) return null;

        var substitutedImage = image.getImageWithSubstitutedColorSpace(factory, replacementColorSpace);
        if (substitutedImage == null || IDOMImage.isEqual(image, substitutedImage)) return null;
        return substitutedImage;
    }

    private static IDOMColorSpace? GetReplacementDeviceColorSpace(IEDLClassFactory factory, IDOMImage image)
    {
        using var frame = image.getImageFrame(factory);
        using var sourceColorSpace = frame.getColorSpace();
        return GetReplacementDeviceColorSpace(factory, sourceColorSpace);
    }

    private static IDOMColorSpace? GetReplacementDeviceColorSpace(
        IEDLClassFactory factory,
        IDOMColorSpace? sourceColorSpace)
    {
        if (sourceColorSpace == null ||
            sourceColorSpace.getColorSpaceType() != IDOMColorSpace.eColorSpaceType.eICCBased) return null;

        using var iccBasedColorSpace = IDOMColorSpaceICCBased.fromRCObject(sourceColorSpace);
        if (iccBasedColorSpace == null) return null;

        var processChannels = IColorManager.get(factory).getNumComponentsForICCBasedSpace(iccBasedColorSpace);
        return processChannels switch
        {
            1 => IDOMColorSpaceDeviceGray.create(factory),
            3 => IDOMColorSpaceDeviceRGB.create(factory),
            4 => IDOMColorSpaceDeviceCMYK.create(factory),
            _ => null
        };
    }

    private sealed class RemoveEmbeddedProfilesTransform(IEDLClassFactory factory)
        : ICustomTransform.IImplementation
    {
        public override IDOMImage transformImage(
            ICustomTransform.IImplementation genericImplementation,
            IDOMImage image,
            CTransformState state) =>
            SubstituteImageColorSpace(factory, image) ??
            genericImplementation.transformImage(null, image, state);

        public override IDOMColorSpace transformColorSpace(
            ICustomTransform.IImplementation genericImplementation,
            IDOMColorSpace colorSpace,
            CTransformState state) =>
            GetReplacementDeviceColorSpace(factory, colorSpace) ??
            genericImplementation.transformColorSpace(null, colorSpace, state);
    }
}
