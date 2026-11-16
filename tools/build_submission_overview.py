"""Builds SUBMISSION_OVERVIEW.png, a one screen contact sheet of the
submission document.

The pages are found by looking for the headings in the PDF, so the sheet keeps
itself in step when the document changes.

    python tools\\build_submission_overview.py
"""

import os
import re

import pymupdf
from PIL import Image, ImageDraw

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PDF = os.path.join(ROOT, "COS202 Project Report Olisaemeka Chinedu-Eze.pdf")
OUT = os.path.join(ROOT, "docs", "SUBMISSION_OVERVIEW.png")

BG = (36, 38, 44)
FG = (238, 238, 240)
MUTED = (178, 182, 190)
ACCENT = (120, 190, 255)
BORDER = (78, 82, 92)

COLS = 4
CELL_W = 470
LABEL_H = 34
GAP = 16


def find_page(document, needle, start=0):
    for index in range(start, document.page_count):
        if needle in document[index].get_text():
            return index + 1
    return None


def build_pages(document):
    """The pages worth showing, with their caption."""
    pages = []

    pages.append((1, "Cover, details and table of content"))

    source = find_page(document, "Part 1: Source Files/Code", 1)
    if source:
        pages.append((source, "Part 1 - Source Files/Code"))

    report = find_page(document, "Part 2: Project Report", 1)
    if report:
        for offset in range(3):
            number = report + offset
            if number <= document.page_count:
                pages.append((number, "Part 2 - Project Report %d/3" % (offset + 1)))

    shots = find_page(document, "Part 3: Screenshots of the program", 1)
    if shots:
        for offset in range(3):
            number = shots + offset
            if number <= document.page_count:
                pages.append((number, "Part 3 - Screenshots"))

    uml = find_page(document, "Part 4: UML/Class Diagram", 1)
    if uml:
        pages.append((uml, "Part 4 - UML/Class Diagram"))

    key = find_page(document, "How the diagram shows each required relationship", 1)
    if key:
        pages.append((key, "UML key: the four relationships"))

    return pages


def main():
    document = pymupdf.open(PDF)
    pages = build_pages(document)
    if not os.path.isdir(os.path.dirname(OUT)):
        os.makedirs(os.path.dirname(OUT))

    tiles = []
    for number, caption in pages:
        page = document[number - 1]
        zoom = CELL_W / page.rect.width
        pixmap = page.get_pixmap(matrix=pymupdf.Matrix(zoom, zoom))
        image = Image.frombytes("RGB", (pixmap.width, pixmap.height), pixmap.samples)
        tiles.append((number, caption, image))

    tile_h = max(image.height for _, _, image in tiles)
    rows = (len(tiles) + COLS - 1) // COLS

    sheet_w = GAP + COLS * (CELL_W + GAP)
    sheet_h = GAP + 76 + rows * (tile_h + LABEL_H + GAP)

    sheet = Image.new("RGB", (sheet_w, sheet_h), BG)
    draw = ImageDraw.Draw(sheet)

    draw.text((GAP + 4, 18), "SCAPMS - submission overview", fill=FG)
    draw.text((GAP + 4, 44),
              "COS202 Project Report Olisaemeka Chinedu-Eze.pdf  |  %d pages  |  source index, three page report, six screenshots, UML diagram and key"
              % document.page_count,
              fill=MUTED)

    for index, (number, caption, image) in enumerate(tiles):
        column = index % COLS
        row = index // COLS
        x = GAP + column * (CELL_W + GAP)
        y = GAP + 76 + row * (tile_h + LABEL_H + GAP)

        draw.rectangle((x - 1, y - 1, x + CELL_W + 1, y + tile_h + 1), outline=BORDER)
        sheet.paste(image, (x, y))

        draw.text((x + 2, y + tile_h + 8), "p. %d" % number, fill=ACCENT)
        draw.text((x + 46, y + tile_h + 8), caption, fill=FG)

    sheet.save(OUT)
    print("wrote %s  (%d tiles, %dx%d)" % (OUT, len(tiles), sheet.width, sheet.height))


if __name__ == "__main__":
    main()

