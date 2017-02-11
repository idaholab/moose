#!/usr/bin/env python
import os
import re
import unittest
import MooseDocs
from MooseDocs.testing import MarkdownTestCase

class TestDiagramExtension(MarkdownTestCase):
    """
    Test that the MooseDiagram extension for using dot-language is working.
    """

    REGEX = r'<img class="moose-diagram" src="media/tmp_(.*?)\.moose\.svg" style="background:transparent; border:0px" />'

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
