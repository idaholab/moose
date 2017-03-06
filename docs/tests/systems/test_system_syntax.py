#!/usr/bin/env python
import os
import unittest
import MooseDocs
from MooseDocs.testing import MarkdownTestCase

class TestMooseObjectSyntax(MarkdownTestCase):
    """
    Test commands in MooseObjectSyntax extension.
    """
    def testSubObjects(self):
        md = '!subobjects /Adaptivity/Markers'
        html = self.convert(md)
        self.assertIn('<h2 id="available-sub-objects">Available Sub-Objects</h2>', html)
        self.assertIn('<div class="collapsible-header moose-group-header">Framework Objects</div>', html)
        loc = os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'content', 'documentation', 'systems', 'Adaptivity', 'Markers', 'framework', 'BoxMarker.md')
        self.assertIn('<div class="moose-collection-name col l4"><a href="{}">BoxMarker</a></div>'.format(loc), html)

    def testSubObjectsTitle(self):
        md = '!subobjects /Adaptivity/Markers title=My Custom Title'
        html = self.convert(md)
        self.assertIn('<h2 id="my-custom-title">My Custom Title</h2>', html)

    def testSubObjectsError(self):
        md = '!subobjects /Not/A/Valid/System'
        html = self.convert(md)
        self.assertIn('<p></p>', html)

    def testSubSystems(self):
        md = '!subsystems /Adaptivity'
        html = self.convert(md)
        self.assertIn('<h2 id="available-sub-systems">Available Sub-Systems</h2>', html)
        self.assertIn('<div class="collapsible-header moose-group-header">Framework Systems</div>', html)
        loc = os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'content', 'documentation', 'systems', 'Adaptivity', 'Markers', 'index.md')
        self.assertIn('<div class="moose-collection-name col l4"><a href="{}">Markers</a></div>'.format(loc), html)

    def testSubSystemsTitle(self):
        md = '!subsystems /Adaptivity title=My Custom Title'
        html = self.convert(md)
        self.assertIn('<h2 id="my-custom-title">My Custom Title</h2>', html)

    def testSubSystemsError(self):
        md = '!subsystems /Not/A/Valid/System'
        html = self.convert(md)
        self.assertIn('<p></p>', html)

if __name__ == '__main__':
    unittest.main(verbosity=2)
