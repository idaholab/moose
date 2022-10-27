#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
import unittest
import yaml
import subprocess # for assertRaises
import mooseutils

MOOSE_DIR = mooseutils.git_root_dir()
YAML_FILE = os.path.relpath(os.path.join(MOOSE_DIR, "scripts", "module_hash.yaml"))

# TODO: replace with a hash from master branch which contains the updated module_hash.yaml file
# where the following keys in HASH_TABLE.keys() exist (at time of writing we are missing mpi, moose).
COMMIT = 'FIXME'

# The following represent the current hashes for COMMIT
HASH_TABLE = {'mpich'     : 'd385d66',
              'petsc'     : '662b9e2',
              'libmesh'   : 'c095106',
              'moose'     : 'cd87adc'}

try:
    with open(f'{YAML_FILE}', 'r', encoding='utf-8') as rc_file:
        RAW = yaml.safe_load(rc_file)
        ENTITIES = RAW['packages']
        LIBRARIES = RAW['zip_keys']

except FileNotFoundError:
    print(f'fatal: {YAML_FILE} not found')
    sys.exit(1)

except yaml.scanner.ScannerError:
    print(f'fatal: {YAML_FILE} parsing error')
    sys.exit(1)

sys.path.insert(0, os.path.join(MOOSE_DIR, 'scripts'))
from module_hash import main

@classmethod
def setUpClass(cls):
    cls.flag = False

class Test(unittest.TestCase):
    def read_yaml(self):
        # Read the module_hash.yaml file at 'COMMIT' time in history
        out = mooseutils.check_output(['git', 'show',
                                       f'{COMMIT}:./{os.path.basename(YAML_FILE)}'],
                                      cwd=os.path.join(MOOSE_DIR, 'scripts'))
        return yaml.safe_load(out)

    def testExpectedOutputArbitrary(self):
        # Verify a non existent hash generates 'arbitrary'
        for library in LIBRARIES:
            out = mooseutils.check_output(['./module_hash.py', library, 'abc1234', '--quiet'],
                                          cwd=os.path.join(MOOSE_DIR, 'scripts'))
            self.assertEqual('arbitrary\n', out)

        # Verify a non existent hash generates a warning, and prints 'arbitrary'
        for library in LIBRARIES:
            out = mooseutils.check_output(['./module_hash.py', library, 'abc1234'],
                                          cwd=os.path.join(MOOSE_DIR, 'scripts'))
            self.assertIn('warning: commit abc1234 is not a valid git commit\narbitrary\n', out)

        # Verify a non existent hash fatally errors when invoked with --influential
        for library in LIBRARIES:
            with self.assertRaises(subprocess.CalledProcessError) as cm:
                out = mooseutils.check_output(['./module_hash.py', library, 'abc1234', '--influential'],
                                              cwd=os.path.join(MOOSE_DIR, 'scripts'))
            e = cm.exception
            self.assertIn('warning: commit abc1234 is not a valid git commit\narbitrary\n', out)

    def testExpectedOutputHASH(self):
        # TODO: When module_hash.yaml has entered git history, uncomment the following test
        #       and replace COMMIT='FIXME' with a proper hash containing module_hash.yaml
        # # Verify a vetted hash reports correctly for all involved libraries
        # for library in LIBRARIES:
        #     out = mooseutils.check_output(['./module_hash.py', library, COMMIT, '--quiet'],
        #                                   cwd=os.path.join(mooseutils.git_root_dir(), 'scripts'))
        #     self.assertIn(HASH_TABLE[library], out)
        return

if __name__ == '__main__':
    unittest.main(verbosity=2, buffer=True)
