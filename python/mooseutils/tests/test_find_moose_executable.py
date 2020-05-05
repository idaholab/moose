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
import shutil
import unittest
import tempfile
import mooseutils

class Test(unittest.TestCase):
    def testFindWithName(self):
        tmp_dir = tempfile.mkdtemp()
        tmp_exe = os.path.join(tmp_dir, 'app-opt')
        with open(tmp_exe, 'w') as fid:
            fid.write('foo')

        os.environ['METHOD'] = 'opt'
        exe = mooseutils.find_moose_executable(tmp_dir, name='app')
        self.assertEqual(exe, tmp_exe)

        shutil.rmtree(tmp_dir)

    def testFindWithMakefile(self):
        tmp_dir = tempfile.mkdtemp()
        tmp_exe = os.path.join(tmp_dir, 'app_test-opt')
        tmp_mkf = os.path.join(tmp_dir, 'Makefile')
        with open(tmp_exe, 'w') as fid:
            fid.write('foo')
        with open(tmp_mkf, 'w') as fid:
            fid.write('APPLICATION_NAME := app_test')

        os.environ['METHOD'] = 'opt'
        exe = mooseutils.find_moose_executable(tmp_dir)
        self.assertEqual(exe, tmp_exe)

    def testFindWithDir(self):
        tmp_dir = tempfile.mkdtemp()
        tmp_exe = os.path.join(tmp_dir, os.path.basename(tmp_dir) + '-opt')
        with open(tmp_exe, 'w') as fid:
            fid.write('foo')

        os.environ['METHOD'] = 'opt'
        exe = mooseutils.find_moose_executable(tmp_dir)
        self.assertEqual(exe, tmp_exe)

    def testFindWithMultiple(self):
        tmp_dir = tempfile.mkdtemp()
        tmp_exe = os.path.join(tmp_dir, 'app_test-opt')
        tmp_mkf = os.path.join(tmp_dir, 'Makefile')
        with open(tmp_exe, 'w') as fid:
            fid.write('foo')
        with open(tmp_mkf, 'w') as fid:
            fid.write('APPLICATION_NAME := other_app\nAPPLICATION_NAME := app_test')

        os.environ['METHOD'] = 'opt'
        exe = mooseutils.find_moose_executable(tmp_dir)
        self.assertEqual(exe, tmp_exe)

if __name__ == '__main__':
    unittest.main(verbosity=2, buffer=True)
