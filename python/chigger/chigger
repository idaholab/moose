#!/usr/bin/env python
import sys
import argparse
import chigger

def commandLineOptions():
    """
    Command-line options for chigger tool.
    """
    parser = argparse.ArgumentParser(description="Command-line utility for working with chigger visualization tools.")
    subparser = parser.add_subparsers(title='Commands', help='Chigger utility to utilize.', dest='command')

    exodus_parser = subparser.add_parser('info', help='Operate on a ExodusII file.')
    exodus_parser.add_argument('filename', type=str, help="The ExodusII file to open.")
    return vars(parser.parse_args())

def exodus_info(filename=None, **kwargs):
    """
    Display information about the supplied ExodusII file.
    """
    reader = chigger.exodus.ExodusReader(filename)
    print reader

if __name__ == '__main__':

    options = commandLineOptions()
    cmd = options.pop('command')
    if cmd == 'info':
        retcode = exodus_info(**options)
    else:
        retcode = 1

    sys.exit(retcode)
