from PIL import Image

img = Image.open("cursor.png").convert('RGBA')
raw = bytearray()

i = 0
for y in range(img.height):
    for x in range(img.width):
        p = img.getpixel((x, y))
        raw.append(p[2]) # b
        raw.append(p[1]) # g
        raw.append(p[0]) # r
        raw.append(p[3]) # a
        i += 1

output = open("cursor.raw", "wb")
output.write(raw)
output.close()
