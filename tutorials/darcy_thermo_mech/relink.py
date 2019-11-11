#!/usr/bin/env python
import os
import subprocess
import argparse

parser = argparse.ArgumentParser(description='Process some integers.')
parser.add_argument('from_dir', type=str, help="Directory linking from.")
parser.add_argument('to_dir', type=str, help="Directory linking to")
args = parser.parse_args()

link_from = os.path.abspath(args.from_dir)
link_to = os.path.abspath(args.to_dir)
for root, _, files in os.walk(link_from):
    for fname in files:
        if not fname.endswith(('.C', '.h', '.i', 'tests')):
            continue

        full_link_from = os.path.join(root, fname)
        full_link_to = full_link_from.replace(link_from, link_to)

        if (os.path.islink(full_link_from)):
            cmd = ['ln', '-sf', os.path.relpath(full_link_to, os.path.dirname(full_link_from)), fname]
            print(' '.join(cmd))
            #subprocess.call(cmd, cwd=root)
