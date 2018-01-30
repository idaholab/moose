#!/usr/bin/env python
#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
from MooseDocs.testing import MarkdownTestCase

class TestGlobalExtension(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.global']

    @classmethod
    def updateExtensions(cls, configs):
        """
        Add a import filename.
        """
        configs['MooseDocs.extensions.global']['import'] = \
            ['python/MooseDocs/tests/globals/test_import.yml']

    def testGlobalConfig(self):
        md = '[libMesh]'
        html = self.convert(md)
        self.assertIn('<a href="http://libmesh.github.io/">libMesh</a>', html)

    def testImportConfig(self):
        md = '[Google]'
        html = self.convert(md)
        self.assertIn('<a href="http://www.google.com">Google</a>', html)

if __name__ == '__main__':
    unittest.main(verbosity=2)
