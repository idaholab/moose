#!/usr/bin/env python
import sys
import os

# Locate MOOSE directory
MOOSE_DIR = os.getenv('MOOSE_DIR', os.path.join(os.getcwd(), 'moose'))
if not os.path.exists(MOOSE_DIR):
    MOOSE_DIR = os.path.join(os.getenv('HOME'), 'projects', 'moose')
if not os.path.exists(MOOSE_DIR):
    raise Exception('Failed to locate MOOSE, specify the MOOSE_DIR environment variable.')

# Append MOOSE python directory
sys.path.append(os.path.join(MOOSE_DIR, 'python'))
import MooseDocs

if __name__ == '__main__':
    sys.exit(MooseDocs.moosedocs())
