import os
import inspect
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

    # Define the local directory
    cls._path = os.path.abspath(os.path.dirname(inspect.getfile(cls)))

    # Create the markdown object
    cwd = os.getcwd()
    os.chdir(os.path.join(MooseDocs.MOOSE_DIR, 'docs'))

    config = MooseDocs.yaml_load('moosedocs.yml')

    extensions, extension_configs = MooseDocs.get_markdown_extensions(config)
    cls.parser = markdown.Markdown(extensions=extensions, extension_configs=extension_configs)
    os.chdir(cwd)

  def assertTextFile(self, name):
    """
    Assert method for comparing converted html (text) against the text in gold file.

    Inputs:
        name[str]: Name of html file to open, there should be a corresponding file in the gold directory.
    """
    # Read text file
    self.assertTrue(os.path.exists(name), "Failed to locate test output file: {}".format(name))
    with open(name) as fid:
        text = fid.read().encode('utf-8').splitlines()

    # Read gold file
    gold_name = os.path.join(os.path.dirname(name), 'gold', os.path.basename(name))
    self.assertTrue(os.path.exists(gold_name), "Failed to locate gold file: {}".format(gold_name))
    with open(gold_name) as fid:
      gold = fid.read().encode('utf-8').splitlines()

    # Compare
    result = list(difflib.ndiff(gold, text))
    n =  len(max(result, key=len))
    msg = "\nThe supplied text differs from the gold ({}) as follows:\n{}\n{}\n{}".format(gold_name, '~'*n, '\n'.join(result).encode('utf-8'), '~'*n)
    self.assertTrue(text==gold, msg)

  def assertConvert(self, name, md):
    """
    Assert that markdown is converted to html, compared against gold file.

    Inputs:
        name[str]: The name of the html file to create.
        md[str]: The markdown text to convert and create.
    """
    name = os.path.join(self._path, name)
    with open(name, 'w') as fid:
        fid.write(self.parser.convert(md))
    self.assertTextFile(name)

  def assertConvertFile(self, name, md_name):
    """
    Assert that markdown file is converted to html, compared against gold file.

    Inputs:
        name[str]: The name of the html file to create.
        md_name[str]: The markdown text file to convert and create.
    """

    # Read the markdown file
    md_name = os.path.join(self._path, md_name)
    self.assertTrue(os.path.exists(md_name), "Failed to locate markdown file: {}".format(md_name))
    with open(md_name, 'r') as fid:
        md = fid.read()

    # Parse the markdown
    name = os.path.join(self._path, name)
    with open(name, 'w') as fid:
        fid.write(self.parser.convert(md))

    # Compare against gold
    self.assertTextFile(name)
