#!/usr/bin/python3
import sys
import __future__

def temperature_decode(txt):
    if txt == '7fff':
        return None
    return int(txt, 16) / 10.0

def voltage_decode(txt):
    if txt == 'ff':
        return None
    return int(txt, 16) / 10.0


def decode(data):
    length = len(data)
    print(data[0:2])
    print(data[2:6])
    print(data[6:10])
    if length != 10:
        raise Exception("Bad data length, min 10 characters expected")
    return {
        "pir_count": int(data[6:10], 16),
        "temperature": temperature_decode(data[2:6]),
        "voltage": voltage_decode(data[0:2])
    }


def pprint(data):
    print('PIR count :', data['pir_count'])
    print('Voltage :', data['voltage'], 'V')
    print('Temperature :', data['temperature'], '°C')


if __name__ == '__main__':
    if len(sys.argv) != 2 or sys.argv[1] in ('help', '-h', '--help'):
        print("usage: python3 decode.py [data]")
        print("example decode paket: python3 decode.py 0123456789")
        exit(1)

    data = decode(sys.argv[1])
    pprint(data)