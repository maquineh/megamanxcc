from pathlib import Path
from random import Random
from struct import pack
from zlib import compress, crc32


OUTPUT = Path(__file__).resolve().parents[1] / 'res'

SKY = ('222044', '221c3c', '34264c', '493050', '65405a', '85506a', 'a66169', 'cd7771',
       'ffac71', 'ffe0a1', '784a67', '433851', '24394e', '274d60', '376874', '588084')
METAL = ('000000', '151a31', '202a41', '29394c', '304b5b', '365f70', '467e87', '6ba8a3',
         '9bc8b6', 'd6dbb3', '62404a', 'a36554', 'd88f59', 'ffca73', '347070', '546073')
ACTOR = ('000000', '202037', '483348', '785255', 'a85448', 'cc6744', 'f79850', 'ffd276',
         'ffedb1', '254754', '3f8491', '71c4c4', 'b1ebe0', 'fbffdf', '6a516b', 'a48591')


class Canvas:
    def __init__(self, width, height):
        self.width, self.height = width, height
        self.rows = [bytearray(width) for _ in range(height)]

    def box(self, x, y, w, h, color):
        for row in self.rows[y:y + h]:
            row[x:x + w] = bytes([color]) * w

    def paste(self, other, x):
        for dest, source in zip(self.rows, other.rows):
            dest[x:x + other.width] = source

    def save(self, name, palette):
        def chunk(kind, data):
            return pack('>I', len(data)) + kind + data + pack('>I', crc32(kind + data))

        pixels = b''.join(b'\0' + row for row in self.rows)
        header = pack('>IIBBBBB', self.width, self.height, 8, 3, 0, 0, 0)
        palette_data = b''.join(bytes.fromhex(color) for color in palette)
        (OUTPUT / name).write_bytes(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', header)
                                     + chunk(b'PLTE', palette_data) + chunk(b'IDAT', compress(pixels))
                                     + chunk(b'IEND', b''))


def backdrop():
    canvas = Canvas(512, 224)
    for y, height, color in ((0, 32, 1), (32, 20, 2), (52, 18, 3), (70, 20, 4),
                             (90, 22, 5), (112, 20, 6), (132, 24, 7), (156, 68, 13)):
        canvas.box(0, y, 512, height, color)

    for y in range(73, 145):
        span = int((36 * 36 - (y - 109) ** 2) ** .5)
        canvas.box(169 - span, y, 2 * span + 1, 1, 9 if y < 113 else 8)
    for y, x, width, color in ((42, 26, 46, 3), (44, 33, 74, 3), (63, 293, 73, 4),
                               (66, 312, 68, 4), (116, 142, 69, 7), (125, 130, 91, 7),
                               (133, 18, 88, 6), (137, 389, 87, 6)):
        canvas.box(x, y, width, 2, color)

    # Each distant contour wraps at the same height at both ends of the plane.
    for ridge, color in (([145, 126, 147, 140, 147, 130, 136, 103, 145], 10),
                         ([148, 141, 153, 150, 139, 148, 152, 146, 148], 11)):
        for x in range(512):
            section, step = divmod(x, 64)
            y = ridge[section] + (ridge[section + 1] - ridge[section]) * step // 64
            canvas.box(x, y, 1, 157 - y, color)

    # Distant tidal turbines and their gantries.
    for x, y in ((60, 116), (286, 109), (419, 120)):
        canvas.box(x, y, 3, 157 - y, 12)
        canvas.box(x - 10, y - 1, 24, 2, 12)
        canvas.box(x, y - 10, 2, 12, 12)
        canvas.box(x + 2, y + 2, 2, 6, 12)
        canvas.box(x - 4, 150, 13, 7, 12)

    rng = Random(31)
    for y in range(158, 224, 3):
        for _ in range(9):
            width = rng.randrange(4, 25)
            x = rng.randrange(0, 512 - width)
            canvas.box(x, y, width, 1, rng.choice((12, 14, 15)))
    canvas.save('estuary.png', SKY)


def metal_tiles():
    sheet = Canvas(96, 8)
    for index in range(12):
        tile = Canvas(8, 8)
        tile.box(0, 0, 8, 8, 3)
        if index < 5:
            tile.box(0, 0, 8, 1, 9)
            tile.box(0, 1, 8, 2, 7)
            tile.box(0, 3, 8, 1, 5)
            tile.box(0, 4, 8, 2, 4)
            tile.box(0, 7, 8, 1, 1)
            if index in (1, 2, 3):
                tile.box(0 if index == 1 else 6 if index == 2 else 2, 1, 2 if index != 3 else 4, 4, 12)
            if index == 4:
                tile.box(0, 0, 8, 3, 13)
                for y in range(3):
                    tile.box(5 - y, y, 2, 1, 10)
        else:
            tile.box(0, 0, 8, 1, 4)
            tile.box(0, 7, 8, 1, 2)
            if index in (5, 6):
                edge = 0 if index == 5 else 6
                tile.box(edge, 0, 2, 8, 5)
                tile.box(edge, 2, 1, 1, 8)
            if index == 7:
                tile.box(1, 2, 6, 4, 1)
                tile.box(2, 3, 4, 2, 5)
            if index == 8:
                tile.box(0, 3, 8, 2, 10)
                tile.box(1, 3, 2, 1, 12)
            if index == 9:
                tile.box(0, 2, 8, 1, 1)
                tile.box(0, 5, 8, 1, 1)
                tile.box(1, 3, 5, 1, 6)
            if index == 10:
                tile.box(2, 0, 4, 8, 2)
                tile.box(3, 1, 1, 6, 5)
            if index == 11:
                tile.box(0, 1, 8, 5, 1)
                tile.box(2, 2, 4, 3, 14)
        sheet.paste(tile, index * 8)
    sheet.save('metal.png', METAL)


def sprites():
    sheet = Canvas(120, 32)
    for frame, feet in enumerate(((7, 13), (5, 15), (9, 11), (12, 6), (8, 14))):
        actor = Canvas(24, 32)
        airborne = frame == 4
        for x in feet:
            y = 23 if airborne else 26
            actor.box(x, 21, 4, y - 18, 3)
            actor.box(x, y, 6, 3, 1)
            actor.box(x + 1, y, 4, 1, 6)
        actor.box(4, 12, 4, 9, 9)
        actor.box(4, 15, 2, 4, 11)
        actor.box(7, 12, 11, 12, 1)
        actor.box(8, 13, 9, 9, 5)
        actor.box(9, 13, 5, 4, 7)
        actor.box(8, 4, 11, 9, 1)
        actor.box(9, 3, 8, 2, 6)
        actor.box(9, 5, 9, 4, 7)
        actor.box(10, 5, 3, 1, 8)
        actor.box(13, 8, 7, 4, 9)
        actor.box(15, 8, 5, 2, 12)
        actor.box(15, 16, 8, 5, 1)
        actor.box(16, 17, 6, 3, 10)
        actor.box(21, 17, 2, 2, 12)
        sheet.paste(actor, frame * 24)
    sheet.save('runner.png', ACTOR)

    shot = Canvas(8, 8)
    shot.box(0, 3, 8, 2, 10)
    shot.box(2, 2, 5, 4, 11)
    shot.box(3, 3, 5, 2, 13)
    shot.save('pulse.png', ACTOR)

    target = Canvas(16, 16)
    target.box(3, 1, 10, 14, 1)
    target.box(1, 3, 14, 10, 1)
    target.box(3, 3, 10, 10, 4)
    target.box(4, 3, 8, 1, 7)
    target.box(5, 5, 6, 6, 7)
    target.box(7, 6, 2, 4, 8)
    target.box(3, 12, 10, 1, 3)
    target.save('target.png', ACTOR)


if __name__ == '__main__':
    backdrop()
    metal_tiles()
    sprites()
