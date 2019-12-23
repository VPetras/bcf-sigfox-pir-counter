#!/usr/bin/python3
import sys
import __future__

def temperature_decode(txt):
    if txt == 'ff':
        return None
    return int(txt, 16) / 2.0 - 28


def decode(data):
    length = len(data)
    if length < 4:
        raise Exception("Bad data length, min 10 characters expected")
    return {
        "pir_count": int(data[2:4], 16),
        "temperature": temperature_decode(data[0:2]),
        "voltage": (int(data[4], 16) / 4.0)
    }


def pprint(data):
    print('PIR count :', data['pir_count'])
    print('Voltage :', data['voltage'], 'V')
    print('Temperature :', data['temperature'], '°C')


if __name__ == '__main__':
    if len(sys.argv) != 2 or sys.argv[1] in ('help', '-h', '--help'):
        print("usage: python3 decode.py [data]")
        print("example ALERT MOTION paket: python3 decode.py 01234")
        exit(1)

    data = decode(sys.argv[1])
    pprint(data)