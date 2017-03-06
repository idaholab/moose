import os
import inspect
import unittest
import difflib
import markdown
import MooseDocs

def text_diff(text, gold):
    """
    Helper for creating nicely formatted text diff message.
    """
    result = list(difflib.ndiff(gold, text))
    n =  len(max(result, key=len))
    msg = "\nThe supplied text differs from the gold as follows:\n{0}\n{1}\n{0}".format('~'*n, '\n'.join(result).encode('utf-8'))
    return msg

class MarkdownTestCase(unittest.TestCase):
    """
    Provides functions for converting markdown to html and asserting conversion against
    gold html files.
    """
    parser = None
    working_dir = os.getcwd()
    TEMPLATE = False


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

        config = MooseDocs.load_config('moosedocs.yml')

        extensions, extension_configs = MooseDocs.get_markdown_extensions(config)
        cls.updateExtensionConfigs(extension_configs)
        cls.parser = MooseDocs.MooseMarkdown(extensions=extensions, extension_configs=extension_configs)
        os.chdir(cwd)

    @classmethod
    def updateExtensionConfigs(cls, extension_configs):
        """
        Method to change the arguments that come from the configuration file for
        specific tests.  This way one can test optional arguments without permanently
        changing moosedocs.yml
        """
        if 'testBibtexMacro' in dir(cls):
            if 'MooseDocs.extensions.MooseMarkdownExtension' in extension_configs:
                extension_configs['MooseDocs.extensions.MooseMarkdownExtension']['macro_files'] =\
                  ['docs/bib/macro_test_abbrev.bib']

        extension_configs['MooseDocs.extensions.MooseMarkdownExtension']['template'] = cls.TEMPLATE

    def setUp(self):
        """
        Always work from the 'docs' directory
        """
        os.chdir(os.path.join(MooseDocs.MOOSE_DIR, 'docs'))

    def tearDown(self):
        """
        Restore the working directory.
        """
        os.chdir(self.working_dir)

    def readGold(self, name):
        """
        Read gold file in current directory.
        """
        gold_name = os.path.join(os.path.dirname(name), 'gold', os.path.basename(name))
        self.assertTrue(os.path.exists(gold_name), "Failed to locate gold file: {}".format(gold_name))
        with open(gold_name) as fid:
            gold = fid.read().encode('utf-8').splitlines()
        return gold

    def convert(self, md):
        """
        Convenience function for converting markdown to html.
        """
        return self.parser.convert(md)

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
        gold = self.readGold(name)
        self.assertEqual(text, gold, text_diff(text, gold))

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

class TestLatexBase(unittest.TestCase):
    """
    Test that basic html to latex conversion working.
    """
    working_dir = os.getcwd()
    defaults = vars(MooseDocs.command_line_options(['latex', 'fake.md']))

    def setUp(self):
        """
        Runs prior to each test.
        """
        os.chdir(os.path.join(MooseDocs.ROOT_DIR, 'docs'))

    def tearDown(self):
        """
        Runs after each test.
        """
        os.chdir(self.working_dir)

    def assertLaTeX(self, md, gold, preamble='', **kwargs):
        """
        Assert that the supplied markdown can be converted to latex.

        This mimics latex.py without creating files.

        Inputs:
            md[str]: The markdown to convert.
            gold[str]: The expected latex.
        """
        kwargs.update(self.defaults)
        config_file = os.path.join(MooseDocs.ROOT_DIR, 'docs', kwargs['config_file'])
        html, settings = MooseDocs.html2latex.generate_html(md, config_file)
        for key, value in kwargs.iteritems():
            if not value and key in settings:
                kwargs[key] = settings[key]
        tex, h2l = MooseDocs.html2latex.generate_latex(html, **kwargs)
        self.assertEqual(tex, gold)
        self.assertEqual(h2l.preamble(), preamble)
