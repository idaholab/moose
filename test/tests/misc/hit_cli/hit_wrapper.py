#!/usr/bin/env python3

import sys
import os
import shutil
import subprocess

if len(sys.argv) < 2:
    print("Usage:\nhit_wrapper.py gold_output [hit arguments]\n")

CWD = os.path.dirname(os.path.realpath(__file__))
MOOSE_DIR = os.getenv('MOOSE_DIR', os.path.abspath(os.path.join(CWD, '..', '..', '..', '..')))

# in place compiled hit
hit = os.path.join(MOOSE_DIR, 'framework', 'contrib', 'hit', 'hit')
if not os.path.exists(hit):
    # installed hit
    hit = os.path.abspath(os.path.join(CWD, '../../../../../../bin/hit'))
if not os.path.exists(hit):
    print('Failed to locate hit executable.')
    sys.exit(1)

gold_file = sys.argv[1]
hit_args = sys.argv[2:]

with open(gold_file, 'rb') as f:
    gold = f.read()

command = [hit] + hit_args
print("Running: ", ' '.join(command))

p = subprocess.Popen(command, shell=False, stdout=subprocess.PIPE)
p.wait()
print("hit returned ", p.returncode)
out = p.communicate()[0]

if out == gold:
    print("Passed.")
    sys.exit(0)

print("Output:\n", out, "\ndoes not match gold:\n", gold)
print(sys.version)
sys.exit(1)
