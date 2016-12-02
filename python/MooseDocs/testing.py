import os
import unittest
import difflib
import markdown
import MooseDocs

class MarkdownTestCase(unittest.TestCase):
  """
  Provides functions for converting markdown to html and asserting conversion against
  gold html files.
  """
  parser = None

  @classmethod
  def setUpClass(cls):
    """
    Create the markdown parser using the 'moosedocs.yml' configuration file.
    """

    cwd = os.getcwd()
    os.chdir(os.path.join(MooseDocs.MOOSE_DIR, 'docs'))

    config = MooseDocs.yaml_load('moosedocs.yml')

    extensions, extension_configs = MooseDocs.get_markdown_extensions(config)
    cls.parser = markdown.Markdown(extensions=extensions, extension_configs=extension_configs)
    os.chdir(cwd)

  def assertTextFile(self, name, text):
    """
    Assert method for comparing converted html (text) against the text in gold file.
    """
    # Read gold file
    gold_name = os.path.join('gold', name)
    self.assertTrue(os.path.exists(gold_name), "Failed to locate gold file: {}".format(gold_name))
    with open(gold_name) as fid:
      gold = fid.read().encode('utf-8').splitlines()

    # Compare
    text = text.splitlines()
    result = list(difflib.ndiff(gold, text))
    n =  len(max(result, key=len))
    msg = "\nThe supplied text differs from the gold ({}) as follows:\n{}\n{}\n{}".format(gold_name, '~'*n, '\n'.join(result).encode('utf-8'), '~'*n)
    self.assertTrue(text==gold, msg)

  def assertConvert(self, name, md):
    """
    Assert that markdown is converted to html, compared against gold file.
    """
    with open(name, 'w') as fid:
        fid.write(self.parser.convert(md))
    self.assertTextFile(name, self.parser.convert(md))
