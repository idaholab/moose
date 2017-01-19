import os
import re
import MooseDocs
from MooseDocs.testing import MarkdownTestCase

class TestDiagramExtension(MarkdownTestCase):
  """
  Test that the MooseDiagram extension for using dot-language is working.
  """

  REGEX = r'<img class="moose-diagram" src="media/tmp_(.*?)\.moose\.svg" style="background:transparent; border:0px" />'
  working_dir = os.getcwd()

  def testGraph(self):
    os.chdir(os.path.join(MooseDocs.MOOSE_DIR, 'docs'))
    md = 'graph{bgcolor="#ffffff00";a -- b -- c;b -- d;}'
    html = self.parser.convert(md)
    match = re.search(self.REGEX, html)
    self.assertTrue(match != None)
    os.chdir(self.working_dir)

  def testDirGraph(self):
    os.chdir(os.path.join(MooseDocs.MOOSE_DIR, 'docs'))
    md = 'digraph{a -> b;b -> c;c -> d;d -> a;}'
    html = self.parser.convert(md)
    match = re.search(self.REGEX, html)
    self.assertTrue(match != None)
    os.chdir(self.working_dir)
