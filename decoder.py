#!/usr/bin/python3
import sys
import __future__

def temperature_decode(txt):
    if txt == 'ff':
        return None
    return int(txt, 16) / 10.0

def voltage_decode(txt):
    if txt == 'f':
        return None
    return int(txt, 16) / 10.0


def decode(data):
    length = len(data)
    if length != 5:
        raise Exception("Bad data length, min 10 characters expected")
    return {
        "pir_count": int(data[3:5], 16),
        "temperature": temperature_decode(data[1:3]),
        "voltage": voltage_decode(data[0:1])
    }


def pprint(data):
    print('PIR count :', data['pir_count'])
    print('Voltage :', data['voltage'], 'V')
    print('Temperature :', data['temperature'], '°C')


if __name__ == '__main__':
    if len(sys.argv) != 2 or sys.argv[1] in ('help', '-h', '--help'):
        print("usage: python3 decode.py [data]")
        print("example decode paket: python3 decode.py 01234")
        exit(1)

    data = decode(sys.argv[1])
    pprint(data)