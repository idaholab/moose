import os
import subprocess
import markdown

from MooseTextFile import MooseTextFile
from MooseImageFile import MooseImageFile
from MooseInputBlock import MooseInputBlock
from MooseCppMethod import MooseCppMethod
from MoosePackageParser import MoosePackageParser
from MooseMarkdownLinkPreprocessor import MooseMarkdownLinkPreprocessor
from MooseCarousel import MooseCarousel
from MooseDiagram import MooseDiagram
from MooseSlidePreprocessor import MooseSlidePreprocessor
import MooseDocs
import utils

class MooseMarkdown(markdown.Extension):
    """
    Extensions that comprise the MOOSE flavored markdown.
    """

    def __init__(self, *args, **kwargs):

        # Determine the root directory via git
        root = os.path.dirname(subprocess.check_output(['git', 'rev-parse', '--git-dir'], stderr=subprocess.STDOUT))

        # Define the configuration options
        self.config = dict()
        self.config['root'] = [root, "The root directory of the repository, if not provided the root is found using git."]
        self.config['make'] = [root, "The location of the Makefile responsible for building the application."]
        self.config['repo'] = ['', "The remote repository to create hyperlinks."]
        self.config['docs_dir'] = [os.path.join('docs', 'content'), "The location of the markdown to be used for generating the site."]
        self.config['slides'] = [False, "Enable the parsing for creating reveal.js slides."]
        self.config['package'] = [False, "Enable the use of the MoosePackageParser."]
        self.config['graphviz'] = ['/opt/moose/graphviz/bin', 'The location of graphviz executable for use with diagrams.']

        # Define the directory where the markdown is contained, which will be searched for auto link creation
        self._markdown_database_dir = os.path.join(self.config['root'][0], self.config['docs_dir'][0])

        # Construct the extension object
        super(MooseMarkdown, self).__init__(*args, **kwargs)

    def extendMarkdown(self, md, md_globals):
        """
        Builds the extensions for MOOSE flavored markdown.
        """

        # Strip description from config
        config = dict()
        for key, value in self.config.iteritems():
            config[key] = value[0]

        # Prepcoessors
        md.preprocessors.add('moose_auto_link', MooseMarkdownLinkPreprocessor(self._markdown_database_dir, markdown_instance=md), '_begin')
        if config['slides']:
            md.preprocessors.add('moose_slides', MooseSlidePreprocessor(markdown_instance=md), '_end')

        # Block processors
        md.parser.blockprocessors.add('diagrams', MooseDiagram(md.parser, graphviz=config['graphviz']), '_begin')
        md.parser.blockprocessors.add('slideshow', MooseCarousel(md.parser, root=config['root']), '_begin')

        # Inline Patterns
        md.inlinePatterns.add('moose_input_block', MooseInputBlock(markdown_instance=md, repo=config['repo'], root=config['root']), '<image_link')
        md.inlinePatterns.add('moose_cpp_method', MooseCppMethod(markdown_instance=md, make=config['make'], repo=config['repo'], root=config['root']), '<image_link')
        md.inlinePatterns.add('moose_text', MooseTextFile(markdown_instance=md, repo=config['repo'], root=config['root']), '<image_link')
        md.inlinePatterns.add('moose_image', MooseImageFile(markdown_instance=md, root=config['root']), '<image_link')

        if config['package']:
            md.inlinePatterns.add('moose_package_parser', MoosePackageParser(markdown_instance=md), '_end')

def makeExtension(*args, **kwargs):
    return MooseMarkdown(*args, **kwargs)
