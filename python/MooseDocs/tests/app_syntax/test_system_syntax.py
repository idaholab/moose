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

import os
import unittest
import MooseDocs
from MooseDocs.testing import MarkdownTestCase

class TestMooseObjectSyntax(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.app_syntax']

    def testSubObjects(self):
        md = '!subobjects /Adaptivity/Markers'
        html = self.convert(md)
        self.assertIn('<h2>Available Sub-Objects</h2>', html)
        self.assertIn('<div class="collapsible-header moose-group-header">Framework Objects' \
                      '</div>', html)
        loc = os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'content', 'documentation', 'systems',
                           'Adaptivity', 'Markers', 'framework', 'BoxMarker.md')
        self.assertIn('<div class="moose-collection-name col l4"><a href="{}">BoxMarker</a></div>' \
                      .format(loc), html)

    def testSubObjectsTitle(self):
        md = '!subobjects /Adaptivity/Markers title=My Custom Title'
        html = self.convert(md)
        self.assertIn('<h2>My Custom Title</h2>', html)

    def testSubObjectsError(self):
        md = '!subobjects /Not/A/Valid/System'
        html = self.convert(md)
        self.assertIn('<p></p>', html)

    def testSubSystems(self):
        md = '!subsystems /Adaptivity'
        html = self.convert(md)
        self.assertIn('<h2>Available Sub-Systems</h2>', html)
        self.assertIn('<div class="collapsible-header moose-group-header">Framework Systems</div>',
                      html)
        loc = os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'content', 'documentation', 'systems',
                           'Adaptivity', 'Markers', 'index.md')
        self.assertIn('<div class="moose-collection-name col l4"><a href="{}">Markers</a></div>'. \
                      format(loc), html)

    def testSubSystemsTitle(self):
        md = '!subsystems /Adaptivity title=My Custom Title'
        html = self.convert(md)
        self.assertIn('<h2>My Custom Title</h2>', html)

    def testSubSystemsError(self):
        md = '!subsystems /Not/A/Valid/System'
        html = self.convert(md)
        self.assertIn('<p></p>', html)

    def testSystemsList(self):
        md = '!systems'
        html = self.convert(md)
        self.assertIn('<h2 id="adaptivity">Adaptivity', html)
        self.assertIn('<h3 id="adaptivity/markers">Adaptivity/Markers', html)
        self.assertIn('<div class="collapsible-header moose-group-header">Framework Objects</div>',
                      html)
        self.assertIn('Adaptivity/Markers/framework/BoxMarker.md">BoxMarker</a>',
                      html)

if __name__ == '__main__':
    unittest.main(verbosity=2)
