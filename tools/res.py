import struct

from PIL import Image, ImageFont, ImageDraw

from process_states import process_states

# EGA color palette (RGB values)
EGA_PALETTE = [
    (0x00, 0x00, 0x00),  # 0: Black
    (0x00, 0x00, 0xAA),  # 1: Blue
    (0x00, 0xAA, 0x00),  # 2: Green
    (0x00, 0xAA, 0xAA),  # 3: Cyan
    (0xAA, 0x00, 0x00),  # 4: Red
    (0xAA, 0x00, 0xAA),  # 5: Magenta
    (0xAA, 0x55, 0x00),  # 6: Brown
    (0xAA, 0xAA, 0xAA),  # 7: Light Gray
    (0x55, 0x55, 0x55),  # 8: Dark Gray
    (0x55, 0x55, 0xFF),  # 9: Light Blue
    (0x55, 0xFF, 0x55),  # 10: Light Green
    (0x55, 0xFF, 0xFF),  # 11: Light Cyan
    (0xFF, 0x55, 0x55),  # 12: Light Red
    (0xFF, 0x55, 0xFF),  # 13: Light Magenta
    (0xFF, 0xFF, 0x55),  # 14: Yellow
    (0xFF, 0xFF, 0xFF),  # 15: White
]


def find_closest_ega_color(r, g, b, a):
    """Find the closest EGA color index for given RGB values."""
    min_dist = float('inf')
    closest = 0
    if a != 255:
        return -1

    for i, (er, eg, eb) in enumerate(EGA_PALETTE):
        dist = (r - er) ** 2 + (g - eg) ** 2 + (b - eb) ** 2
        if dist < min_dist:
            min_dist = dist
            closest = i
    return closest


def pixels_from_image(img):
    pixels = []
    for y in range(img.size[1]):
        row = []
        for x in range(img.size[0]):
            p = img.getpixel((x, y))
            row.append(find_closest_ega_color(p[0], p[1], p[2], p[3]))
        pixels.append(row)

    return pixels

def shift_row(row, shift, transparent_index):
    return [transparent_index for _ in range(shift)] + row + [transparent_index for _ in range(8 - shift)]

def get_shifted_pixels(pixels, shift, transparent_index, crop_end=0):
    new_pixels = []
    for row in pixels:
        shifted_row = shift_row(row, shift, transparent_index)

        if crop_end != 0:
            shifted_row = shifted_row[:-crop_end]

        new_pixels.append(shifted_row)

    return new_pixels

def load_bmp(filename):
    """Load a BMP file using Pillow and return pixel data."""
    img = Image.open(filename)

    if img.mode != 'RGBA':
        img = img.convert('RGBA')

    # if img.size != (16, 16):
    #     raise ValueError("Image must be 16x16 pixels")

    # Get pixel data
    return pixels_from_image(img)

def convert_subregion(ega_indices, left, top, width, height, transparent_index=-1, write_mask=False, num_color_planes=4):
    # print(f"convert_subregion: {left},{top},{width},{height}")
    mask_data = bytearray()

    planes = [bytearray(), bytearray(), bytearray(), bytearray()]

    if write_mask:
        if width != 8:
            raise "Width must be 8 for subregion in order to create mask"

        for row in ega_indices[top:top+height]:
            byte_val = 0
            row2 = row[left:left+width]
            for x in range(8):
                if row2[7-x] != transparent_index:
                    byte_val |= (1 << x)

            byte_val = ~byte_val & 0xff
            # print('write mask', byte_val)
            mask_data.append(byte_val)

    for row in ega_indices[top:top+height]:
        row2 = row[left:left + width]
        for plane_idx in range(num_color_planes):
            byte_val = 0
            for x in range(width):
                color_val = row2[x]
                if color_val == transparent_index:
                    color_val = 0
                # Extract bit for this plane
                bit = (color_val >> plane_idx) & 1
                byte_val |= (bit << (7 - (x % 8)))

                # Every 8 pixels, store the byte
                if (x % 8) == 7:
                    planes[plane_idx].append(byte_val)
                    byte_val = 0

    result = bytearray()

    # Return in BGRI order: Blue, Green, Red, Intensity
    for i in range(num_color_planes):
        result += planes[i]

    if write_mask:
        result += mask_data

    return result

def convert_tile(input_file, output_file):
    print("Converting to planar EGA format (BGRI)...")
    pixels = load_bmp(input_file)

    final_data = bytearray()

    for y in range(0, len(pixels), 16):
        for x in range(0, len(pixels[0]), 16):
            data = convert_subregion(pixels, x, y, 16, 16, transparent_index=-1, write_mask=False)
            final_data += data

    print(f"Writing to {output_file}...")
    with open(output_file, 'wb') as f:
        f.write(final_data)

    print(f"Done! Output size: {len(final_data)} bytes")
    print(f"Expected: {16 * 4 * 2} bytes (16 rows × 4 planes × 2 bytes per row)")

def convert_sprite(input_file, output_file, transparent_index=-1):
    print(f"Convert Sprite: {input_file} -> {output_file}")

    pixels = load_bmp(input_file)
    width = len(pixels[0])
    height = len(pixels)
    print(f"Size: {width} x {height}")
    # Convert 8x8 sub-regions to EGA data, so the final data is several 40-byte chunks

    # if transparent_index == -1:
    #     transparent_index = pixels[0][0]

    print(f"Transparent index: {transparent_index}")
    final_data = bytearray()
    for x in range(0, len(pixels[0]), 8):
        data = convert_subregion(pixels, x, 0, 8, len(pixels), transparent_index=transparent_index, write_mask=True)
        final_data += data

    with open(output_file, 'wb') as f:
        f.write(struct.pack('<H', width >> 3))
        f.write(struct.pack('<H', height))
        f.write(final_data)

def get_font_char_data(pixels):
    final_data = bytearray()

    for y in range(0, len(pixels), 8):
        for x in range(0, len(pixels[0]), 8):
            data = convert_subregion(pixels, x, y, 8, 8, transparent_index=-1, write_mask=True, num_color_planes=1)
            final_data += data

    return final_data

def process_font(input_file, output_file_ega, output_file_png):
    atlas = Image.new('RGBA', (128, 48), (0, 0, 0, 0))
    draw = ImageDraw.Draw(atlas)
    font = ImageFont.truetype(input_file, 6)
    draw.fontmode = "1"
    for i in range(32, 128):
        char = chr(i)
        bbox = font.getbbox(char)
        # width = bbox[2] - bbox[0]
        # height = bbox[3] - bbox[1]
        idx = (i - 32)
        row = idx // (128 / 8)
        col = idx % (128 / 8)
        # draw.text((col * 8 + 1, row * 8 + 1), f"{char}", font=font, fill=(0, 0, 0, 255), stroke_width=0)
        draw.text((col * 8, row * 8), f"{char}", font=font, fill=(255, 255, 255, 255), stroke_width=0)

    # atlas.show()
    pixels = pixels_from_image(atlas)

    final_data = bytearray()
    final_data += get_font_char_data(pixels)
    final_data += get_font_char_data(get_shifted_pixels(pixels, 4, -1, 8))


    with open(output_file_ega, 'wb') as f:
        # f.write(struct.pack('<H', width >> 3))
        # f.write(struct.pack('<H', height))
        f.write(final_data)

    atlas.save(output_file_png, "png")


def main():
    process_font("/home/matt/Downloads/my 3x5 tiny mono pixel font.ttf",
                 "/home/matt/dos/tg2/data/font.ega",
                 "/home/matt/dos/tg2/dev/font.png")

    #return
    convert_tile("dev/tile.png", "data/tile.ega")
    # convert_file("/home/matt/dos/ega/sprite.png", "/home/matt/dos/ega/sprite.ega", write_mask=True)
    # convert_sprite("/home/matt/dos/ega/sprite2.png", "/home/matt/dos/tg2/sprite2.ega", transparent_index=13)
    # convert_sprite("/home/matt/dos/ega/sprite3.png", "/home/matt/dos/tg2/sprite3.ega", transparent_index=13)
    convert_sprite("dev/player.png", "data/player.ega")
    for i in range(2, 7):
        convert_sprite(f"dev/player{i}.png", f"data/player{i}.ega")
    convert_sprite("dev/monster.png", "data/monster.ega")
    convert_sprite("dev/cursor.png", "data/cursor.ega")

    process_states()

if __name__ == "__main__":
    main()