#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
"""Generate a VGUI1 bitmap font sheet (32-bit RLE TGA) from a TTF font.

The sheet format expected by vgui's FontPlat_Bitmap (see
docs/spec/FontSheetTGA.md): a horizontal strip of 256 equal cells,
sheet width = 256 * cellWide, sheet height = cell height. Character N
occupies columns [N*cellWide, (N+1)*cellWide). A pixel is "lit" when
any of its RGBA bytes is nonzero; the per-char advance is derived from
the rightmost lit column, so glyphs must be rendered flush-left.

Usage: ttf2vguifont.py <font.ttf> <pixel_size> <out.tga> [bearing_chars]

bearing_chars: characters whose natural left side bearing is preserved in
the cell instead of being flushed left. Use it for glyphs that are drawn
as overlays at one pen position (e.g. Marlett's checkbox "cdefg"+"ab" and
radio "hijklmn" part sets) — they only align through their bearings, and
their advance is never consumed. Everything else stays flush-left so the
derived advance (rightmost lit column + 1) stays tight; baked-in bearings
inflate advances and perturb text layout (TextImage starts wrapping).
"""

import io
import struct
import sys

from fontTools.ttLib import TTFont
from fontTools.ttLib.tables._c_m_a_p import CmapSubtable
from PIL import Image, ImageDraw, ImageFont

THRESHOLD = 128  # sheet is 1-bit; alias anti-aliased coverage at 50%


def load_truetype(path, size):
    """Load a TTF with PIL, remapping Windows symbol cmaps (3,0) that
    FreeType would otherwise ignore (e.g. Wine's marlett.ttf) down to
    plain char codes. Returns (font, mapped_codepoints)."""
    tt = TTFont(path)

    symbol = next((t for t in tt["cmap"].tables
                   if t.platformID == 3 and t.platEncID == 0), None)
    if symbol is None:
        mapped = {code for code in tt.getBestCmap() if code < 0x100}
        return ImageFont.truetype(path, size), mapped

    unicode_cmap = CmapSubtable.newSubtable(4)
    unicode_cmap.platformID = 3
    unicode_cmap.platEncID = 1
    unicode_cmap.language = 0
    unicode_cmap.cmap = {code & 0xFF: name for code, name in symbol.cmap.items()}
    tt["cmap"].tables = [unicode_cmap]

    buf = io.BytesIO()
    tt.save(buf)
    buf.seek(0)
    return ImageFont.truetype(buf, size), set(unicode_cmap.cmap)


def render_glyphs(font, mapped, size, bearing_chars):
    """Render mapped chars, returning (glyphs, cell_wide, cell_tall).
    Each glyph is (bitmap, left, top). Glyphs are flushed left (tight
    advance) unless listed in bearing_chars, which keep their natural
    left side bearing so overlaid composite parts line up."""
    pad = size * 2  # headroom for bearings around the draw origin
    boxes = {}

    for ch in sorted(c for c in mapped if 0x20 < c < 0x100):
        canvas = Image.new("L", (pad * 2, pad * 2), 0)
        ImageDraw.Draw(canvas).text((pad, pad), chr(ch), font=font, fill=255)
        canvas = canvas.point(lambda v: 255 if v >= THRESHOLD else 0)
        bbox = canvas.getbbox()
        if bbox:
            # negative bearings can't be represented (cell x=0 is the pen)
            left = max(0, bbox[0] - pad) if chr(ch) in bearing_chars else 0
            boxes[ch] = (canvas.crop(bbox), left, bbox[1])

    if not boxes:
        sys.exit("no glyphs rendered — empty or unmapped font?")

    top = min(t for _, _, t in boxes.values())
    bottom = max(t + g.height for g, _, t in boxes.values())
    cell_tall = bottom - top
    glyphs = {ch: (g, l, t - top) for ch, (g, l, t) in boxes.items()}
    cell_wide = max(l + g.width for g, l, _ in glyphs.values()) + 2

    return glyphs, cell_wide, cell_tall


def write_tga(path, sheet):
    """Write a bottom-up 32-bit RLE TGA (type 10, descriptor 0) matching
    the flavor of the Half-Life font sheets."""
    wide, tall = sheet.size
    pixels = sheet.load()

    def bgra(x, y):
        return b"\xff\xff\xff\xff" if pixels[x, y] else b"\x00\x00\x00\x00"

    out = bytearray(struct.pack("<BBBHHBHHHHBB",
                                0, 0, 10,   # no id, no colormap, RLE truecolor
                                0, 0, 0,    # colormap spec
                                0, 0,       # origin
                                wide, tall,
                                32, 0))     # bpp, descriptor (bottom-up)

    for y in reversed(range(tall)):  # bottom-up row order
        x = 0
        while x < wide:
            run = 1
            while (x + run < wide and run < 128
                   and pixels[x + run, y] == pixels[x, y]):
                run += 1
            out.append(0x80 | (run - 1))
            out += bgra(x, y)
            x += run

    with open(path, "wb") as f:
        f.write(out)


def main():
    if len(sys.argv) not in (4, 5):
        sys.exit(__doc__)

    ttf, size, out = sys.argv[1], int(sys.argv[2]), sys.argv[3]
    bearing_chars = sys.argv[4] if len(sys.argv) == 5 else ""

    font, mapped = load_truetype(ttf, size)
    glyphs, cell_wide, cell_tall = render_glyphs(font, mapped, size, bearing_chars)

    sheet = Image.new("L", (cell_wide * 256, cell_tall), 0)
    for ch, (glyph, left, top) in glyphs.items():
        sheet.paste(glyph, (ch * cell_wide + left, top))

    write_tga(out, sheet)
    print(f"{out}: 256 cells of {cell_wide}x{cell_tall}, "
          f"{len(glyphs)} glyphs, sheet {cell_wide * 256}x{cell_tall}")


if __name__ == "__main__":
    main()
