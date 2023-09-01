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
import logging
from MooseDocs import common, base
from MooseDocs.common import exceptions
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, tagging
logging.basicConfig()

class TestTagging(MooseDocsTestCase):
    EXTENSIONS = [core, command, tagging]

    def setupExtension(self, ext):
        if ext == tagging:
            return dict(active=True, allowed_keys=['application', 'foo', 'simulation_type', 'fiscal_year'], js_file='tagging.js')

    def setupContent(self):
        config = [dict(root_dir='python/MooseDocs/test/content', content=['js/tagging.js'])]
        return common.get_content(config, '.md')

    def testInit(self):
        """
        Test tagging extension initialization
        """
        with self.assertLogs(level=logging.WARNING) as cm:
            self._MooseDocsTestCase__setup()
        self.assertEqual(len(cm.output), 1)
        self.assertIn('The tagging extension is experimental!', cm.output[0])

if __name__ == '__main__':
    unittest.main(verbosity=2)

