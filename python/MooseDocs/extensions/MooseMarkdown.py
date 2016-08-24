import os
import subprocess
import markdown

from MooseTextFile import MooseTextFile
from MooseInputBlock import MooseInputBlock
from MooseCppMethod import MooseCppMethod
from MoosePackageParser import MoosePackageParser
from MooseMarkdownLinkPreprocessor import MooseMarkdownLinkPreprocessor
from MooseSlideTreeprocessor import MooseSlideTreeprocessor
import MooseDocs
import utils


class MooseMarkdown(markdown.Extension):

    def __init__(self, *args, **kwargs):

        root = os.path.dirname(subprocess.check_output(['git', 'rev-parse', '--git-dir'], stderr=subprocess.STDOUT))

        self.config = dict()
        self.config['root'] = [root, "The root directory of the repository, if not provided the root is found using git."]
        self.config['make'] = [root, "The location of the Makefile responsible for building the application."]
        self.config['repo'] = ['', "The remote repository to create hyperlinks."]
        self.config['docs_dir'] = [os.path.join('docs', 'content'), "The location of the markdown to be used for generating the site."]

        self._markdown_database_dir = os.path.join(self.config['root'][0], self.config['docs_dir'][0])
        super(MooseMarkdown, self).__init__(*args, **kwargs)

    def extendMarkdown(self, md, md_globals):

        # Strip description from config
        config = dict()
        for key, value in self.config.iteritems():
            config[key] = value[0]

        # Preprocessors
        #md.treeprocessors.add('moose_slides', MooseSlideTreeprocessor(md), '_end')
        md.preprocessors.add('moose_auto_link', MooseMarkdownLinkPreprocessor(md, self._markdown_database_dir), '>include')

        # Inline Patterns
        md.inlinePatterns.add('moose_input_block', MooseInputBlock(config), '<image_link')
        md.inlinePatterns.add('moose_cpp_method', MooseCppMethod(config), '<image_link')
        md.inlinePatterns.add('moose_source', MooseTextFile(config), '<image_link')
        md.inlinePatterns.add('moose_package_parser', MoosePackageParser(config), '<image_link')

def makeExtension(*args, **kwargs):
    return MooseMarkdown(*args, **kwargs)
