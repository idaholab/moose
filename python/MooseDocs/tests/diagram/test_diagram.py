#!/usr/bin/env python
#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################

import re
import unittest
from MooseDocs.testing import MarkdownTestCase

class TestDiagramExtension(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.diagram']
    REGEX = r'<img class="moose-diagram" src="media/tmp_(.*?)\.moose\.svg" ' \
             'style="background:transparent; border:0px" />'

    def testGraph(self):
        md = 'graph{bgcolor="#ffffff00";a -- b -- c;b -- d;}'
        html = self.parser.convert(md)
        match = re.search(self.REGEX, html)
        self.assertTrue(match != None)

    def testDirGraph(self):
        md = 'digraph{a -> b;b -> c;c -> d;d -> a;}'
        html = self.parser.convert(md)
        match = re.search(self.REGEX, html)
        self.assertTrue(match != None)

if __name__ == '__main__':
    unittest.main(verbosity=2)
