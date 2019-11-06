#!/bin/python3

import os


def bytes2ui(data):

    data_len = len(data)
    value = 0
    for b in data:
        value >>= 8
        value |= (b << ((data_len-1) * 8))
    return value


def norm_sample(sample, bits):

    max_ui = 1 << bits
    max_positive = max_ui >> 1
    sample_int = sample if sample < max_positive else sample - max_ui
    sample_float = sample_int / max_positive
    return sample_float


def get_attributes(raw_file_data):

    data = raw_file_data
    f_size = len(data)
    attributes = {}

    if (data[0:4] == b'RIFF') and (data[8:12] == b'WAVE'):
        data_length = bytes2ui(data[4:8])
        if f_size != (data_length + 8):
            print('Incorrect WAV file size')
            return None
    else:
        print('Incorrect WAV file header')
        return None

    if (data[12:16] == b'fmt ') and (bytes2ui(data[16:20]) == 16):
        attributes['format_tag'] = bytes2ui(data[20:22])
        attributes['channels'] = bytes2ui(data[22:24])
        attributes['sample_rate'] = bytes2ui(data[24:28])
        attributes['byte_rate'] = bytes2ui(data[28:32])
        attributes['frame_size'] = bytes2ui(data[32:34])
        attributes['sample_size'] = bytes2ui(data[34:36])
    else:
        print('Incorrect WAV format header')
        return None

    return attributes


def get_mono_data(raw_file_data, frame_size_byte, sample_size_bit):

    frame_size = frame_size_byte
    sample_size = sample_size_bit
    data = raw_file_data
    f_size = len(data)

    if data[36:40] == b'data':
        data_length = bytes2ui(data[40:44])
        if f_size < (data_length + 44):
            print('Incorrect WAV file size')
            return None
    else:
        print('Incorrect WAV file data header')
        return None

    d_out = []

    sample_size_byte = sample_size // 8
    for i in range(data_length // frame_size):
        frame_begin = 44 + (frame_size * i)
        frame_end = frame_begin + frame_size
        frame = data[frame_begin:frame_end]
        sample = norm_sample(bytes2ui(frame[0:sample_size_byte]), sample_size)
        d_out.append(sample)

    return d_out


def mono2ui8(data_in):

    data_out = []

    for m in data_in:
        b = round(m*127 + 128)
        if b > 255:
            b = 255
        if b < 0:
            b = 0
        data_out.append(b)

    return data_out


def center(data_in):

    avg = 0
    for s in data_in:
        avg += s
    avg /= len(data_in)
    data_out = []
    for s in data_in:
        data_out.append(s - avg)

    return data_out


def norm_volume(data_in):

    v_min = min(data_in)
    v_max = max(data_in)
    abs_max = abs(v_min) if abs(v_min) > abs(v_max) else abs(v_max)
    multi = 1 / abs_max
    data_out = []
    for s in data_in:
        data_out.append(s * multi)

    return data_out


def plot(data, sample_rate, title=None):

    t = []
    t_step = 1 / sample_rate

    for i in range(len(data)):
        t.append(t_step * i)

    from matplotlib import pyplot

    pyplot.figure()
    pyplot.plot(t, mono)
    pyplot.xlabel('t [s]')
    if title is not None:
        pyplot.title(title)
    pyplot.grid(True)
    pyplot.show()


def save2c_header(file_name):

    header = os.path.basename(file_name).upper().replace('.', '_')

    f = open(file_name, 'w')
    f.write("#ifndef __{0}__\n#define __{0}__\n\n".format(header))
    f.close()


def save2c_data(file_name, data, name, sample_rate=8000):

    f = open(file_name, 'a')
    f.write('const int {0}_sample_rate = {1};\nconst int {0}_length = {2};\n\n'.format(name, sample_rate, len(data)))
    f.write('const uint8_t {}_data[] PROGMEM = {}\n'.format(name, '{'))

    cnt = 0
    for d in data:
        cnt += 1
        f.write(' {:3d},{}'.format(d, '' if ((cnt & 0xF) != 0) else '\n'))
    f.write(' };\n\n')

    f.close()


def save2c_footer(file_name):

    f = open(file_name, 'a')
    f.write("#endif\n")
    f.close()


if __name__ == "__main__":

    filename = "horn_mono8k.wav"
    f = open(filename, 'br')
    raw_data = f.read()
    f.close()

    horn_atr = get_attributes(raw_data)
    horn_mono = get_mono_data(raw_data, horn_atr['frame_size'], horn_atr['sample_size'])
    horn_mono = center(horn_mono)
    horn_mono = norm_volume(horn_mono)

    horn_ui8_mono = mono2ui8(horn_mono)

    #plot(horn_ui8_mono, horn_atr['sample_rate'], filename)

    filename = "tick_mono8k.wav"
    f = open(filename, 'br')
    raw_data = f.read()
    f.close()

    tick_atr = get_attributes(raw_data)
    tick_mono = get_mono_data(raw_data, tick_atr['frame_size'], tick_atr['sample_size'])
    tick_mono = center(tick_mono)
    tick_mono = norm_volume(tick_mono)

    tick_ui8_mono = mono2ui8(tick_mono)

    #plot(horn_ui8_mono, horn_atr['sample_rate'], filename)

    filename = "tock_mono8k.wav"
    f = open(filename, 'br')
    raw_data = f.read()
    f.close()

    tock_atr = get_attributes(raw_data)
    tock_mono = get_mono_data(raw_data, tock_atr['frame_size'], tock_atr['sample_size'])
    tock_mono = center(tock_mono)
    tock_mono = norm_volume(tock_mono)

    tock_ui8_mono = mono2ui8(tock_mono)

    #plot(horn_ui8_mono, horn_atr['sample_rate'], filename)

    filename = 'sound_data.h'
    save2c_header(filename)
    save2c_data(filename, horn_ui8_mono, 'horn', horn_atr['sample_rate'])
    save2c_data(filename, tick_ui8_mono, 'tick', tick_atr['sample_rate'])
    save2c_data(filename, tock_ui8_mono, 'tock', tock_atr['sample_rate'])
    save2c_footer(filename)
