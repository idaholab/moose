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
#pylint: enable=missing-docstring
import os
import inspect
import unittest
import difflib
import bs4
import MooseDocs
from MooseDocs.html2latex import Translator, BasicExtension, MooseExtension

def text_diff(text, gold):
    """
    Helper for creating nicely formatted text diff message.
    """
    result = list(difflib.ndiff(gold, text))
    n = len(max(result, key=len))
    msg = "\nThe supplied text differs from the gold as follows:\n{0}\n{1}\n{0}" \
         .format('~'*n, '\n'.join(result).encode('utf-8'))
    return msg

class MarkdownTestCase(unittest.TestCase):
    """
    Provides functions for converting markdown to html and asserting conversion against
    gold html files.
    """
    WORKING_DIR = os.getcwd()
    EXTENSIONS = [] # If provided only these extensions will be loaded
    CONFIG = 'website.yml'

    @classmethod
    def setUpClass(cls):
        """
        Create the markdown parser using the configuration file.
        """

        # Define the local directory
        cls._path = os.path.abspath(os.path.dirname(inspect.getfile(cls)))

        # Create the markdown object
        os.chdir(os.path.join(MooseDocs.MOOSE_DIR, 'docs'))

        # Read the YAML configurations
        config = MooseDocs.load_config(cls.CONFIG)

        # Update extension list
        if cls.EXTENSIONS:
            for key in config:
                if key not in cls.EXTENSIONS:
                    config.pop(key)

        cls.updateExtensions(config)
        cls.parser = MooseDocs.MooseMarkdown(extensions=config.keys(), extension_configs=config)
        os.chdir(cls.WORKING_DIR)

    @classmethod
    def updateExtensions(cls, configs):
        """
        Method to change the arguments that come from the configuration file for
        specific tests.  This way one can test optional arguments without permanently
        changing the configuration file.
        """
        pass

    def setUp(self):
        """
        Always work from the 'docs' directory
        """
        os.chdir(os.path.join(MooseDocs.MOOSE_DIR, 'docs'))

    def tearDown(self):
        """
        Restore the working directory.
        """
        os.chdir(self.WORKING_DIR)

    def readGold(self, name):
        """
        Read gold file in current directory.
        """
        gold_name = os.path.join(self.WORKING_DIR, 'gold', os.path.basename(name))
        self.assertTrue(os.path.exists(gold_name),
                        "Failed to locate gold file: {}".format(gold_name))
        with open(gold_name) as fid:
            gold = fid.read().encode('utf-8')
            gold = gold.replace(r'{{CWD}}', os.getcwd())
            gold = gold.replace(r'{{ROOT_DIR}}', MooseDocs.ROOT_DIR)

        return gold.splitlines()

    def convert(self, md_text):
        """
        Convenience function for converting markdown to html.
        """
        return self.parser.convert(md_text)

    def assertMarkdown(self, md_text, gold):
        """
        Convert the supplied markdown and compare the supplied gold.
        """
        html = self.parser.convert(md_text)
        print ('\n{:>15}: {}'*3).format('MARKDOWN', repr(md_text), 'HTML', repr(html),
                                        'HTML (GOLD)', repr(gold))
        self.assertEqual(html, gold, text_diff(html.splitlines(), gold.splitlines()))

    def assertTextFile(self, name):
        """
        Assert method for comparing converted html (text) against the text in gold file.

        Inputs:
            name[str]: Name of html file to open, there should be a corresponding file in the gold
                       directory.
        """
        # Read text file
        self.assertTrue(os.path.exists(name), "Failed to locate test output file: {}".format(name))
        with open(name) as fid:
            text = fid.read().encode('utf-8').splitlines()

        # Read gold file
        gold = self.readGold(name)
        self.assertEqual(text, gold, text_diff(text, gold))

    def assertConvert(self, name, md_text):
        """
        Assert that markdown is converted to html, compared against gold file.

        Inputs:
            name[str]: The name of the html file to create.
            md_texg[str]: The markdown text to convert and create.
        """
        name = os.path.join(self._path, name)
        with open(name, 'w') as fid:
            fid.write(self.parser.convert(md_text))
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
        self.assertTrue(os.path.exists(md_name),
                        "Failed to locate markdown file: {}".format(md_name))
        with open(md_name, 'r') as fid:
            md = fid.read()

        # Parse the markdown
        name = os.path.join(self._path, name)
        with open(name, 'w') as fid:
            fid.write(self.parser.convert(md))

        # Compare against gold
        self.assertTextFile(name)

class TestLatexBase(MarkdownTestCase):
    """
    Test that basic html to latex conversion working.
    """
    EXTENSIONS = [BasicExtension, MooseExtension]
    CONFIG = dict()

    @classmethod
    def setUpClass(cls):
        cls._translator = Translator(extensions=[ext(**cls.CONFIG) for ext in cls.EXTENSIONS])

    def convert(self, html):
        """
        Convert the html to latex.
        """
        return self._translator.convert(html)

    def assertLaTeX(self, html, gold):
        """
        Assert markdown to latex conversion.
        """
        tex = self.convert(html)
        print ('\n{:>15}: {}'*3).format('HTML', repr(html), 'TEX', repr(tex), 'TEX (GOLD)',
                                        repr(gold))
        self.assertEqual(tex, gold, text_diff(tex.splitlines(), gold.splitlines()))

    @staticmethod
    def soup(html_file):
        """
        Assert markdown to latex conversion from an html file.
        """
        with open(html_file, 'r') as fid:
            html = fid.read()
        return bs4.BeautifulSoup(html, 'html.parser')
