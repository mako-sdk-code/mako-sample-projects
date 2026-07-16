/* --------------------------------------------------------------------------------
 * <copyright file="CsvToPdfWithILayout.cs" company="Hybrid Software Helix Ltd">
 *   Copyright (c) 2026 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *   Render semicolon-delimited CSV data as a paginated PDF table. ILayout performs
 *   text shaping, wrapping and measurement; a small table layer supplies the grid,
 *   page breaks, repeated headers, zebra fills and Page X / Y footers.
 *
 *   This example is provided on an "as is" basis and without warranty of any kind.
 *   Hybrid Software Helix Ltd. does not warrant or make any representations
 *   regarding the use or results of use of this example.
 * </summary>
 * --------------------------------------------------------------------------------
 */

using System.Text;
using JawsMako;
using Microsoft.VisualBasic.FileIO;

namespace CsvToPdfWithILayoutCS;

internal static class CsvToPdfWithILayout
{
    // Mako DOM coordinates are XPS units: 96 units per inch.
    private static readonly double PageWidth = Mm(297);  // A4 landscape
    private static readonly double PageHeight = Mm(210);
    private static readonly double TableLeft = Mm(10);
    private static readonly double TableTop = Mm(10);
    private static readonly double HeaderHeight = Mm(9);
    private static readonly double TableBottom = PageHeight - Mm(14);
    private static readonly double CellPadding = Mm(1.2);

    private static readonly double BodyFontSize = Pt(7);
    private const double Leading = 1.2;

    // Predefined widths total 277 mm, exactly the available width between margins.
    private static readonly Column[] Columns =
    [
        new("Id",         Mm(14)),
        new("UserName",   Mm(19)),
        new("Message",    Mm(116)),
        new("TimeStamp",  Mm(30)),
        new("ModuleName", Mm(41.5)),
        new("OldValue",   Mm(28.25)),
        new("NewValue",   Mm(28.25))
    ];

    private static int Main(string[] args)
    {
        if (!TryParseOptions(args, out var options))
        {
            Console.Error.WriteLine(
                "Usage: CsvToPdfWithILayoutCS <input.csv> [output.pdf] [--monospace]");
            return 2;
        }

        var inputPath = Path.GetFullPath(options.InputPath);
        var outputPath = Path.GetFullPath(
            options.OutputPath ?? Path.ChangeExtension(inputPath, ".pdf"));

        try
        {
            Directory.CreateDirectory(Path.GetDirectoryName(outputPath)!);

            // Get the rows from the CSV 
            var rows = ReadCsv(inputPath);

            // Create PDF document 
            using var mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);

            using var assembly = IDocumentAssembly.create(mako);
            using var document = IDocument.create(mako);
            assembly.appendDocument(document);

            var font = options.UseMonospace
                ? FindOpenTypeFont(mako, ["Consolas"])
                : FindOpenTypeFont(mako, ["DejaVu Sans", "Arial"]);
            var pages = new List<IDOMFixedPage>();

            // Add the first page to the document including column headers
            var page = AddPage(mako, document, pages, font);

            for (var rowIndex = 0; rowIndex < rows.Count; rowIndex++)
            {
                using var row = LayoutRow(mako, rows[rowIndex], font);
                var wholeRowHeight = CellPadding * 2 + row.ContentHeight;

                // Keep ordinary rows together. A row higher than a fresh page is
                // divided only at ILayout line-box boundaries and continues below
                // the repeated header on the following page(s).
                if (wholeRowHeight > page.RemainingHeight &&
                    wholeRowHeight <= page.BodyHeight)
                {
                    page = AddPage(mako, document, pages, font);
                }
                else if (wholeRowHeight > page.BodyHeight && !page.IsEmpty)
                {
                    page = AddPage(mako, document, pages, font);
                }

                var firstBand = 0;
                while (firstBand < row.BandCount)
                {
                    var bandsThatFit = row.CountBandsThatFit(
                        firstBand,
                        page.RemainingHeight - CellPadding * 2);

                    if (bandsThatFit < 1)
                    {
                        page = AddPage(mako, document, pages, font);
                        continue;
                    }

                    var sourceHeight = row.GetBandHeight(firstBand, bandsThatFit);
                    var fragmentHeight = CellPadding * 2 + sourceHeight;

                    DrawRowFragment(
                        mako, page.FixedPage, row, rowIndex, page.CursorY,
                        fragmentHeight, firstBand, bandsThatFit);

                    page.CursorY += fragmentHeight;
                    firstBand += bandsThatFit;

                    if (firstBand < row.BandCount)
                        page = AddPage(mako, document, pages, font);
                }
            }

            AddFooters(mako, pages, font);

            using var output = IPDFOutput.create(mako);
            output.setParameter("Producer", "Mako ILayout CSV table example");
            output.setParameter("maxAccumulatedPages", "1");
            output.writeAssembly(assembly, outputPath);

            Console.WriteLine($"Wrote {pages.Count} page(s) to {outputPath}");
            return 0;
        }
        catch (MakoException ex)
        {
            Console.Error.WriteLine("Mako exception: " + ex.m_msg);
            return 1;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine(ex);
            return 1;
        }
    }

    private static bool TryParseOptions(string[] args, out Options options)
    {
        options = new Options("", null, false);
        if (args.Length < 1)
            return false;

        string? outputPath = null;
        var useMonospace = false;

        for (var index = 1; index < args.Length; index++)
        {
            if (args[index].Equals("--monospace", StringComparison.OrdinalIgnoreCase))
            {
                if (useMonospace)
                    return false;

                useMonospace = true;
            }
            else if (outputPath is null)
            {
                outputPath = args[index];
            }
            else
            {
                return false;
            }
        }

        options = new Options(args[0], outputPath, useMonospace);
        return true;
    }

    private static IReadOnlyList<string[]> ReadCsv(string path)
    {
        using var parser = new TextFieldParser(path, Encoding.UTF8, detectEncoding: true);
        parser.TextFieldType = FieldType.Delimited;
        parser.HasFieldsEnclosedInQuotes = true;
        parser.TrimWhiteSpace = false;
        parser.SetDelimiters(";");

        var header = parser.ReadFields();
        if (header is null)
            throw new InvalidDataException("The CSV is empty.");

        var expected = Columns.Select(column => column.Name).ToArray();
        if (!header.SequenceEqual(expected, StringComparer.Ordinal))
        {
            throw new InvalidDataException(
                $"Expected header '{string.Join(';', expected)}' but found " +
                $"'{string.Join(';', header)}'.");
        }

        var rows = new List<string[]>();
        while (!parser.EndOfData)
        {
            var fields = parser.ReadFields() ?? [];
            if (fields.Length != Columns.Length)
            {
                throw new InvalidDataException(
                    $"CSV record {rows.Count + 2} has {fields.Length} fields; " +
                    $"expected {Columns.Length}.");
            }

            rows.Add(fields);
        }

        return rows;
    }

    private static LaidOutRow LayoutRow(IJawsMako mako, string[] fields, Font font)
    {
        var cellNodes = new IDOMNode[Columns.Length];
        var cellBottoms = new double[Columns.Length];
        var glyphBands = new List<VerticalBand>();

        for (var columnIndex = 0; columnIndex < Columns.Length; columnIndex++)
        {
            var text = NormalizeNewlines(fields[columnIndex]);
            var innerWidth = Columns[columnIndex].Width - CellPadding * 2;

            // Worst case is one character per line. The tall frame removes page
            // constraints so ILayout can wrap the complete value in one pass.
            var measurementHeight = Math.Max(
                BodyFontSize * 3, (text.Length + 1) * BodyFontSize * 3);

            using var layout = ILayout.create(mako);
            using var frameBounds = new FRect(0, 0, innerWidth, measurementHeight);
            using var frame = ILayoutFrame.create(
                frameBounds, ILayoutFrame.eVerticalAlignment.eVATop);
            layout.addFrame(frame);

            using var paragraph = ILayoutParagraph.create(
                ILayoutParagraph.eHorizontalAlignment.eHALeft,
                spacingAfter: 0,
                spacingBefore: 0,
                leading: Leading);
            using var run = ILayoutTextRun.create(
                text.Length == 0 ? " " : text,
                font.OpenType,
                font.Index,
                BodyFontSize);
            paragraph.addRun(ILayoutRun.fromRCObject(run.toRCObject()));

            var content = layout.layout(paragraph);
            cellNodes[columnIndex] = content;

            using var glyphNodes =
                content.findChildrenOfType(eDOMNodeType.eDOMGlyphsNode);
            foreach (var glyphNode in glyphNodes.toArray())
            {
                using var glyphs = IDOMGlyphs.fromRCObject(glyphNode.toRCObject());
                using var bounds = glyphs.getBounds(applyTransform: false);
                using var transformState = new CTransformState(glyphs);
                transformState.transform.transformRect(bounds);
                glyphBands.Add(new VerticalBand(bounds.y, bounds.y + bounds.dY));
                cellBottoms[columnIndex] = Math.Max(
                    cellBottoms[columnIndex], bounds.y + bounds.dY);
            }
        }

        return new LaidOutRow(cellNodes, cellBottoms, MakeBreakPositions(glyphBands));
    }

    private static double[] MakeBreakPositions(IEnumerable<VerticalBand> sourceBands)
    {
        var sorted = sourceBands
            .Where(band => band.Bottom > band.Top)
            .OrderBy(band => band.Top)
            .ToArray();

        if (sorted.Length == 0)
            return [0, BodyFontSize];

        // A shaped line can contain several glyph nodes. Merge overlapping vertical
        // bounds so each item represents one actual rendered line, regardless of
        // font fallback or how ILayout divided the run internally.
        var lines = new List<VerticalBand>();
        var current = sorted[0];
        foreach (var candidate in sorted.Skip(1))
        {
            if (candidate.Top <= current.Bottom + 0.01)
            {
                current = new VerticalBand(
                    Math.Min(current.Top, candidate.Top),
                    Math.Max(current.Bottom, candidate.Bottom));
            }
            else
            {
                lines.Add(current);
                current = candidate;
            }
        }
        lines.Add(current);

        var breaks = new double[lines.Count + 1];
        breaks[0] = 0;
        for (var index = 1; index < lines.Count; index++)
        {
            // The midpoint is guaranteed to lie in the whitespace between lines.
            breaks[index] = (lines[index - 1].Bottom + lines[index].Top) / 2;
        }
        breaks[^1] = lines[^1].Bottom;
        return breaks;
    }

    private static PageState AddPage(
        IJawsMako mako,
        IDocument document,
        ICollection<IDOMFixedPage> pages,
        Font headerFont)
    {
        using var page = IPage.create(mako);
        document.appendPage(page);

        var fixedPage = IDOMFixedPage.create(mako, PageWidth, PageHeight);
        page.setContent(fixedPage);
        pages.Add(fixedPage);

        DrawHeader(mako, fixedPage, headerFont);
        return new PageState(fixedPage, TableTop + HeaderHeight, TableBottom);
    }

    private static void DrawHeader(IJawsMako mako, IDOMFixedPage page, Font font)
    {
        var x = TableLeft;

        foreach (var column in Columns)
        {
            using var rect = new FRect(x, TableTop, column.Width, HeaderHeight);
            DrawCellBox(mako, page, rect, 0.97f, 0.97f, 0.97f);

            using var layout = ILayout.create(mako);
            using var textRect = new FRect(
                x + CellPadding,
                TableTop + CellPadding,
                column.Width - CellPadding * 2,
                HeaderHeight - CellPadding * 2);
            using var frame = ILayoutFrame.create(
                textRect, ILayoutFrame.eVerticalAlignment.eVATop);
            layout.addFrame(frame);

            using var run = ILayoutTextRun.create(
                column.Name, font.OpenType, font.Index, BodyFontSize);
            page.appendChild(layout.layout(
                ILayoutRun.fromRCObject(run.toRCObject()),
                ILayoutParagraph.eHorizontalAlignment.eHALeft));

            x += column.Width;
        }
    }

    private static void DrawRowFragment(
        IJawsMako mako,
        IDOMFixedPage page,
        LaidOutRow row,
        int rowIndex,
        double y,
        double fragmentHeight,
        int firstBand,
        int bandCount)
    {
        var shade = rowIndex % 2 == 0 ? 0.91f : 0.985f;
        var x = TableLeft;
        var sourceY = row.Breaks[firstBand];
        var sourceHeight = row.GetBandHeight(firstBand, bandCount);

        for (var columnIndex = 0; columnIndex < Columns.Length; columnIndex++)
        {
            var column = Columns[columnIndex];
            using var cellRect = new FRect(x, y, column.Width, fragmentHeight);
            DrawCellBox(mako, page, cellRect, shade, shade, shade);

            // Do not create a clipped clone when this cell has no text in the
            // continuation band. Besides avoiding needless DOM, this prevents an
            // empty clipped form from obscuring content already drawn on the page.
            if (sourceY >= row.CellBottoms[columnIndex] - 0.01)
            {
                x += column.Width;
                continue;
            }

            // Break positions come from the whitespace between actual ILayout glyph
            // bounds. The clip therefore cannot cut a character or rendered line.
            using var transform = new FMatrix();
            transform.translate(
                x + CellPadding,
                y + CellPadding - sourceY);
            using var clip = IDOMPathGeometry.create(
                mako,
                new FRect(0, sourceY,
                    column.Width - CellPadding * 2, sourceHeight));
            using var group = IDOMGroup.create(mako, transform, clip);
            group.appendChild(row.Cells[columnIndex].cloneTree(mako));
            page.appendChild(group);

            x += column.Width;
        }
    }

    private static void DrawCellBox(
        IJawsMako mako,
        IDOMFixedPage page,
        FRect rect,
        float red,
        float green,
        float blue)
    {
        using var geometry = IDOMPathGeometry.create(mako, rect);
        using var fill = IDOMSolidColorBrush.createSolidRgb(mako, red, green, blue);
        page.appendChild(IDOMPathNode.createFilled(mako, geometry, fill));

        using var border = IDOMSolidColorBrush.createSolidRgb(mako, 0.78f, 0.78f, 0.78f);
        var outline = IDOMPathNode.createStroked(mako, geometry, border);
        outline.setStrokeThickness(0.45);
        page.appendChild(outline);
    }

    private static void AddFooters(IJawsMako mako, IReadOnlyList<IDOMFixedPage> pages, Font font)
    {
        for (var index = 0; index < pages.Count; index++)
        {
            using var layout = ILayout.create(mako);
            using var footerRect = new FRect(
                TableLeft,
                PageHeight - Mm(11),
                PageWidth - TableLeft * 2,
                Mm(6));
            using var frame = ILayoutFrame.create(
                footerRect, ILayoutFrame.eVerticalAlignment.eVACenter);
            layout.addFrame(frame);

            using var run = ILayoutTextRun.create(
                $"{index + 1} / {pages.Count}",
                font.OpenType,
                font.Index,
                BodyFontSize);
            pages[index].appendChild(layout.layout(
                ILayoutRun.fromRCObject(run.toRCObject()),
                ILayoutParagraph.eHorizontalAlignment.eHACenter));
        }
    }

    private static Font FindOpenTypeFont(IJawsMako mako, IEnumerable<string> names)
    {
        foreach (var name in names)
        {
            try
            {
                var font = mako.findFont(name, out var index);
                if (font.getFontType() == IDOMFont.eFontType.eFontTypeOpenType)
                    return new Font(IDOMFontOpenType.fromRCObject(font.toRCObject()), index);
            }
            catch (MakoException)
            {
                // Try the next preferred font.
            }
        }

        throw new InvalidOperationException("No suitable OpenType font was found.");
    }

    private static string NormalizeNewlines(string value) =>
        value.Replace("\r\n", "\n", StringComparison.Ordinal)
             .Replace('\r', '\n');

    private static double Mm(double value) => value / 25.4 * 96.0;

    private static double Pt(double value) => value / 72.0 * 96.0;

    private sealed record Column(string Name, double Width);

    private sealed record Font(IDOMFontOpenType OpenType, uint Index);

    private sealed record Options(string InputPath, string? OutputPath, bool UseMonospace);

    private readonly record struct VerticalBand(double Top, double Bottom);

    private sealed class LaidOutRow(
        IDOMNode[] cells,
        double[] cellBottoms,
        double[] breaks) : IDisposable
    {
        public IDOMNode[] Cells { get; } = cells;
        public double[] CellBottoms { get; } = cellBottoms;
        public double[] Breaks { get; } = breaks;
        public int BandCount => Breaks.Length - 1;
        public double ContentHeight => Breaks[^1] - Breaks[0];

        public double GetBandHeight(int firstBand, int bandCount) =>
            Breaks[firstBand + bandCount] - Breaks[firstBand];

        public int CountBandsThatFit(int firstBand, double availableHeight)
        {
            var count = 0;
            while (firstBand + count < BandCount &&
                   GetBandHeight(firstBand, count + 1) <= availableHeight + 0.01)
            {
                count++;
            }
            return count;
        }

        public void Dispose()
        {
            foreach (var cell in Cells)
                cell.Dispose();
        }
    }

    private sealed class PageState(
        IDOMFixedPage fixedPage,
        double cursorY,
        double tableBottom)
    {
        public IDOMFixedPage FixedPage { get; } = fixedPage;
        public double CursorY { get; set; } = cursorY;
        public double BodyHeight { get; } = tableBottom - (TableTop + HeaderHeight);
        public double RemainingHeight => tableBottom - CursorY;
        public bool IsEmpty => Math.Abs(CursorY - (TableTop + HeaderHeight)) < 0.01;
    }
}
