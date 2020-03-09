#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest

import MooseDocs
from MooseDocs.extensions import core, devel
from MooseDocs import common
from MooseDocs.common import exceptions

class TestLoadExtensions(unittest.TestCase):
    def testLoadFromModule(self):
        ext = common.load_extensions([core])
        self.assertIsInstance(ext, list)
        self.assertIsInstance(ext[0], core.CoreExtension)

    def testLoadFromModuleWithConfig(self):
        ext = common.load_extensions([devel],
                                     {'MooseDocs.extensions.devel':{'test':False}})
        self.assertFalse(ext[0]['test'])

    def testLoadFromStr(self):
        ext = common.load_extensions(['MooseDocs.extensions.core'])
        self.assertIsInstance(ext, list)
        self.assertIsInstance(ext[0], core.CoreExtension)

    def testLoadFromStrWithConfig(self):
        ext = common.load_extensions(['MooseDocs.extensions.devel'],
                                     {'MooseDocs.extensions.devel':{'test':False}})
        self.assertFalse(ext[0]['test'])

    def testMissingMakeExtension(self):
        with self.assertRaises(exceptions.MooseDocsException) as e:
            common.load_extensions([MooseDocs.extensions])
        self.assertIn("does not contain the required 'make_extension'", str(e.exception))

    def testBadModuleName(self):
        with self.assertRaises(exceptions.MooseDocsException) as e:
            common.load_extensions(['not.a.module'])
        self.assertIn("Failed to import the supplied", str(e.exception))

    def testBadModuleType(self):
        with self.assertRaises(exceptions.MooseDocsException) as e:
            common.load_extensions([42])
        self.assertIn("must be a module type", str(e.exception))

if __name__ == '__main__':
    unittest.main(verbosity=2)
