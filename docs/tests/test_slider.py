#!/user/bin/env python
import unittest
from MooseDocs.testing import MarkdownTestCase

class TestSlider(MarkdownTestCase):
  """
  Test to make sure that "moose/python/MooseDocs/extensions/MooseSlider.py"
  parses a test block correctly.
  """

  def testSlider(self):
    md = "!slider max-width=50% left=220px\n" \
         "    docs/media/memory_logger-darkmode.png caption= Output of memory logging tool position=relative left=150px top=-150px\n" \
         "    docs/media/testImage_tallNarrow.png background-color=#F8F8FF caption= This is a tall, thin image color=red font-size=200% width=200px height=100%\n" \
         "    docs/media/github*.png background-color=gray\n" \
         "    docs/media/memory_logger-plot_multi.png"
    self.assertConvert('test_slider.html', md)

if __name__ == '__main__':
  unittest.main(module=__name__, verbosity=2)
