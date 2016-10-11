import os
import re
import subprocess
import markdown
import collections
import yaml
import logging
log = logging.getLogger(__name__)

from MooseObjectSyntax import MooseObjectSyntax
from MooseSystemSyntax import MooseSystemSyntax
from MooseTextFile import MooseTextFile
from MooseImageFile import MooseImageFile
from MooseInputBlock import MooseInputBlock
from MooseCppMethod import MooseCppMethod
from MoosePackageParser import MoosePackageParser
from MooseMarkdownLinkPreprocessor import MooseMarkdownLinkPreprocessor
from MooseCarousel import MooseCarousel
from MooseDiagram import MooseDiagram
from MooseCSS import MooseCSS
from MooseSlidePreprocessor import MooseSlidePreprocessor
from MooseBuildStatus import MooseBuildStatus
from MooseBibtex import MooseBibtex
import MooseDocs
import utils

cache = {'exe' : None,
         'yaml' : None,
         'root' : None,
         'pages' : [],
         'input_files' : collections.OrderedDict(),
         'child_objects': collections.OrderedDict(),
         'syntax': dict()
        }

class MooseMarkdown(markdown.Extension):
    """
    Extensions that comprise the MOOSE flavored markdown.
    """

    def __init__(self, **kwargs):

        # Determine the root directory via git
        global cache
        if not cache['root']:
            cache['root'] = subprocess.check_output(['git', 'rev-parse', '--show-toplevel'], stderr=subprocess.STDOUT).strip('\n')

        # Define the configuration options
        self.config = dict()
        self.config['root'] = [cache['root'], "The root directory of the repository, if not provided the root is found using git."]
        self.config['make'] = [cache['root'], "The location of the Makefile responsible for building the application."]
        self.config['executable'] = ['', "The executable to utilize for generating application syntax."]
        self.config['locations'] = [dict(), "The locations to parse for syntax."]
        self.config['repo'] = ['', "The remote repository to create hyperlinks."]
        self.config['links'] = [dict(), "The set of paths for generating input file and source code links to objects."]
        self.config['docs_dir'] = ['docs', "The location of the documentation directory."]
        self.config['markdown_dir'] = [os.path.join('docs', 'content'), "The location of the markdown to be used for generating the site."]
        self.config['slides'] = [False, "Enable the parsing for creating reveal.js slides."]
        self.config['package'] = [False, "Enable the use of the MoosePackageParser."]
        self.config['graphviz'] = ['/opt/moose/graphviz/bin', 'The location of graphviz executable for use with diagrams.']
        self.config['pages'] = ['pages.yml', "The location of the pages file for auto link creation."]

        # Construct the extension object
        super(MooseMarkdown, self).__init__(**kwargs)

    def extendMarkdown(self, md, md_globals):
        """
        Builds the extensions for MOOSE flavored markdown.
        """

        # Strip description from config
        config = self.getConfigs()

        # Generate YAML data from application
        global cache
        exe = config['executable']
        if exe != cache['exe']:
            if not os.path.exists(exe):
                log.error('The executable does not exist: {}'.format(exe))
            else:
                log.debug("Executing {} to extract syntax.".format(exe))
                raw = utils.runExe(exe, '--yaml')
                cache['exe'] = exe
                cache['yaml'] = utils.MooseYaml(raw)

        # Populate the database for input file and children objects
        if not cache['input_files']:
            log.info('Building input and inheritance databases...')
            for key, path in config['links'].iteritems():
                cache['input_files'][key] =  MooseDocs.database.Database('.i', path, MooseDocs.database.items.InputFileItem, repo=config['repo'])
                cache['child_objects'][key] = MooseDocs.database.Database('.h', path, MooseDocs.database.items.ChildClassItem, repo=config['repo'])

        # Populate the syntax
        if not cache['syntax']:
            for key, value in config['locations'].iteritems():
                cache['syntax'][key] = MooseDocs.MooseApplicationSyntax(cache['yaml'], **value)

        # Populate the pages yaml
        if not cache['pages']:
            yml = MooseDocs.yaml_load(config['pages'])
            for match in re.finditer(r':(.*?\.md)', yaml.dump(yml, default_flow_style=False)):
                cache['pages'].append(match.group(1).strip())

        # Preprocessors
        md.preprocessors.add('moose_bibtex', MooseBibtex(markdown_instance=md, root=config['root']), '_end')
        md.preprocessors.add('moose_auto_link', MooseMarkdownLinkPreprocessor(markdown_instance=md, pages=cache['pages']), '_begin')
        if config['slides']:
            md.preprocessors.add('moose_slides', MooseSlidePreprocessor(markdown_instance=md), '_end')

        # Block processors
        md.parser.blockprocessors.add('diagrams', MooseDiagram(md.parser, root=config['root'], docs_dir=config['docs_dir'], graphviz=config['graphviz']), '_begin')
        md.parser.blockprocessors.add('slideshow', MooseCarousel(md.parser, root=config['root'], docs_dir=config['docs_dir']), '_begin')
        md.parser.blockprocessors.add('css', MooseCSS(md.parser, root=config['root'], docs_dir=config['docs_dir']), '_begin')

        # Inline Patterns
        object_markdown = MooseObjectSyntax(markdown_instance=md,
                                            yaml=cache['yaml'],
                                            syntax=cache['syntax'],
                                            input_files=cache['input_files'],
                                            child_objects=cache['child_objects'],
                                            repo=config['repo'],
                                            root=config['root'],
                                            docs_dir=config['docs_dir'])
        system_markdown = MooseSystemSyntax(markdown_instance=md,
                                            yaml=cache['yaml'],
                                            syntax=cache['syntax'],
                                            root=config['root'],
                                            docs_dir=config['docs_dir'])
        md.inlinePatterns.add('moose_object_syntax', object_markdown, '_begin')
        md.inlinePatterns.add('moose_system_syntax', system_markdown, '_begin')

        md.inlinePatterns.add('moose_input_block', MooseInputBlock(markdown_instance=md, root=config['root'], docs_dir=config['docs_dir'], repo=config['repo']), '<image_link')
        md.inlinePatterns.add('moose_cpp_method', MooseCppMethod(markdown_instance=md, root=config['root'], docs_dir=config['docs_dir'], make=config['make'], repo=config['repo']), '<image_link')
        md.inlinePatterns.add('moose_text', MooseTextFile(markdown_instance=md, root=config['root'], docs_dir=config['docs_dir'], repo=config['repo']), '<image_link')
        md.inlinePatterns.add('moose_image', MooseImageFile(markdown_instance=md, root=config['root'], docs_dir=config['docs_dir']), '<image_link')
        md.inlinePatterns.add('moose_build_status', MooseBuildStatus(markdown_instance=md, root=config['root'], docs_dir=config['docs_dir']), '_begin')

        if config['package']:
            md.inlinePatterns.add('moose_package_parser', MoosePackageParser(markdown_instance=md, root=config['root'], docs_dir=config['docs_dir']), '_end')

def makeExtension(*args, **kwargs):
    return MooseMarkdown(*args, **kwargs)
