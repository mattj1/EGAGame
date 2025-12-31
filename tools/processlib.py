import csv

import requests


def get_lines_csv(path):
    lines = []

    with open(path) as f:
        reader = csv.reader(f)
        next(reader)
        for row in reader:
            elements = [x.strip() for x in row]
            if not elements[0]:
                continue

            lines.append(elements)

            # print(elements)

    return lines


def get_lines(url):
    response = requests.get(url)
    assert response.status_code == 200, 'Wrong status code'
    # print response.content

    lines = [x.decode() for x in response.content.splitlines()[1:]]

    return lines


def palette_from_image(img):

    pal_data = []

    for y in range(0, img.height):
        for x in range(0, img.width):
            col = img.getpixel((x, y))
            if col not in pal_data:
                pal_data.append(col)

    return pal_data