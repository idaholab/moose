#!/usr/bin/env python3
""" Test pyhit """
import sys

if __name__ == '__main__':
    print('\n\n\nTEST loading hit library\n')
    from pyhit import hit
    print(f'Successs: {hit.__file__}')
    sys.exit()
