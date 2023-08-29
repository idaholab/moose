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
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, tagging
logging.basicConfig()

class TestTagging(MooseDocsTestCase):
    EXTENSIONS = [core, command, tagging]

    # test all subcommands simultaneously - each with a unique message
    BRANDS = ['pageName', 'Page_Name', 'Page_5', 'pagecddf_with!7']
    # ICONS = ['report', 'warning', 'comment', 'school']
    MESSAGE = [['k:v ', '  strange:6', 'my_key:val!'],
            ['k:v ', '  strange:6', 'my_key:val!'],
            ['k:v ', '  strange:6', 'my_key:val!'],
            ['k:v ', '  strange:6', 'my_key:val!']]

    TEXT = ['!tagger ' +b+' '+' '.join(m) for b, m in zip(BRANDS, MESSAGE)]

    def testAST(self):
        for i, b in enumerate(self.BRANDS):
            ast = self.tokenize(self.TEXT[i])
            self.assertSize(ast, 0)

            # self.assertToken(ast(0), 'TaggingToken', size=2, brand=b)

            # self.assertToken(ast(0)(0), 'TaggingTitle', brand=b, prefix=True, icon=True, icon_name=self.ICONS[i])
            # self.assertToken(ast(0)(1), 'TaggingContent', size=1, brand=b)


if __name__ == '__main__':
    unittest.main(verbosity=2)

