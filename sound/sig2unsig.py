#!/bin/python3

"""" program converts signed pcm files (output form wav2c "https://github.com/olleolleolle/wav2c") to unsigned
needed by arduino pcm library
:param input .. input file name
:param output .. output file name

example usage:
1) create pcm h file from wav 8kHz mono using wav2c
    wav2c example.wav example_signed.h
2) convert signed wav2c output to unsigned
    sig2unsig.py -i example_signed.h -o example.h
"""

import click


def replace_numbers(numbers_string, offset=127):
    nums = numbers_string.split(',')
    new_numbers = ''
    for n in nums:
        try:
            val = int(n) + offset
            if val > 0xff:
                val = 0xff
            if val < 0:
                val = 0
            new_numbers += '{:3d}, '.format(val)
        except ValueError:
            pass
    return new_numbers


@click.command()
@click.option('--input', '-i', required=True, type=str, help='input file name')
@click.option('--output', '-o', required=True, type=str, help='output file name')
def sig2unsig(input, output):

    fin = input
    fout = open(output, 'w')

    offset = 127

    st = 0

    for line in open(fin):
        new_line = ''
        if st == 0:  # search for word "singed"
            if line.find('signed') != -1:
                st += 1
                line_rep = line.replace('signed', 'unsigned')
                splt = line_rep.split('{')
                new_line += '{}{}'.format(splt[0], '{ ')
                new_line += replace_numbers(splt[1], offset)
        elif st == 1:  # search for curly bracket "}"
            if line.find('}') != -1:
                st += 1
                splt = line.split('}')
                new_line += replace_numbers(splt[0], offset) + '}'
            else:
                new_line += replace_numbers(line, offset)
        if new_line == '':
            new_line = line.strip()
        print(new_line)
        fout.write(new_line + '\n')

    fout.close()


if __name__ == '__main__':
    sig2unsig()
