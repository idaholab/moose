import os
import unittest
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
    self.assertIn('<div class="moose-collection-name col l4"><a href="Adaptivity/Markers/framework/BoxMarker.md">BoxMarker</a></div>', html)

  def testSubObjectsTitle(self):
    md = '!subobjects /Adaptivity/Markers title=My Custom Title'
    html = self.convert(md)
    self.assertIn('<h2 id="my-custom-title">My Custom Title</h2>', html)

  def testSubObjectsError(self):
    md = '!subobjects /Not/A/Valid/System'
    html = self.assertConvert('test_SubObjectsError.html', md)

  def testSubSystems(self):
    md = '!subsystems /Adaptivity'
    html = self.convert(md)
    self.assertIn('<h2 id="available-sub-systems">Available Sub-Systems</h2>', html)
    self.assertIn('<div class="collapsible-header moose-group-header">Framework Systems</div>', html)
    self.assertIn('<div class="moose-collection-name col l4"><a href="systems/Adaptivity/Markers/index.md">Markers</a></div>', html)

  def testSubSystemsTitle(self):
    md = '!subsystems /Adaptivity title=My Custom Title'
    html = self.convert(md)
    self.assertIn('<h2 id="my-custom-title">My Custom Title</h2>', html)

  def testSubSystemsError(self):
    md = '!subsystems /Not/A/Valid/System'
    html = self.assertConvert('test_SubSystemsError.html', md)
