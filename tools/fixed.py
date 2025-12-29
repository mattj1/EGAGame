import math
import struct

def main():

    b = bytearray()

    # 4 bits = 15 - 5 = 10 bits (1024)
    for i in range(0, 32764):
        # 65528 / 2 = 32764
        # This puts a small limit on inputs to the sqrt function

        # print(i / 64, math.sqrt(i / 64), int(math.sqrt(i/64) * 64))
        val = int(math.sqrt(i/16) * 16)

        b += struct.pack("<H", val)
        # 1 = 1/256
        # 2 = 2/256
        # print(int(math.sqrt(i) * 8))
        # print(f'{0}'.format(int(math.sqrt(i * 16))))

    print(len(b))

    with open('data/sqrt.bin', 'wb') as f:
        f.write(b)

if __name__ == '__main__':
    main()