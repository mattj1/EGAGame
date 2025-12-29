import math


def main():
    for i in range(0, 20):

        # print(i / 64, math.sqrt(i / 64), int(math.sqrt(i/64) * 64))
        print(i / 16, math.sqrt(i / 16), int(math.sqrt(i/16) * 16))

        # 1 = 1/256
        # 2 = 2/256
        # print(int(math.sqrt(i) * 8))
        # print(f'{0}'.format(int(math.sqrt(i * 16))))

if __name__ == '__main__':
    main()