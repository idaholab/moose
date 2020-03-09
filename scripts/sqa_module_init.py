#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from __future__ import print_function

import os
import shutil
import argparse

parser = argparse.ArgumentParser(description='Setup SQA documentation for a MOOSE module.')
parser.add_argument('module', type=str, help='The module folder name')
args = parser.parse_args()

folder = args.module
title = folder.replace('_', ' ').title()
doc_location = os.path.join(os.getenv('MOOSE_DIR'), 'modules', folder, 'doc')

# Create YAML config
data = "directories:\n" \
       "    - ${{MOOSE_DIR}}/modules/{}/test/tests\n" \
       "specs:\n" \
       "    - tests\n".format(folder)
ymlfile = os.path.join(doc_location, 'sqa_{}.yml'.format(folder))
with open(ymlfile, 'w+') as fid:
    print('CREATE: {}'.format(ymlfile))
    fid.write(data)

# Copy and update app SQA files
src_dir = os.path.join(os.getenv('MOOSE_DIR'), 'modules', 'tensor_mechanics', 'doc', 'content', 'modules', 'tensor_mechanics', 'sqa')
dst_dir = os.path.join(os.getenv('MOOSE_DIR'), 'modules', folder, 'doc', 'content', 'modules', folder, 'sqa')
sqa_files = ['index.md', 'tensor_mechanics_sdd.md', 'tensor_mechanics_stp.md', 'tensor_mechanics_rtm.md', 'tensor_mechanics_srs.md', 'tensor_mechanics_vvr.md']

if not os.path.isdir(dst_dir):
    os.makedirs(dst_dir)

for fname in sqa_files:
    src = os.path.join(src_dir, fname)
    dst = os.path.join(dst_dir, fname.replace('tensor_mechanics', folder))

    print('COPY: {} -> {}'.format(src, dst))
    shutil.copyfile(src, dst)

    with open(dst, 'r') as fid:
        content = fid.read()
    content = content.replace('category=tensor_mechanics', 'category={}'.format(folder))
    content = content.replace('app=Tensor Mechanics', 'app={}'.format(title))
    with open(dst, 'w') as fid:
        print('UPDATE: {}'.format(dst))
        fid.write(content)
        print(content)
