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
from MooseSlider import MooseSlider
from MooseDiagram import MooseDiagram
from MooseCSS import MooseCSS
from MooseSlidePreprocessor import MooseSlidePreprocessor
from MooseBuildStatus import MooseBuildStatus
from MooseBibtex import MooseBibtex
import MooseDocs
import utils

class MooseMarkdown(markdown.Extension):
  """
  Extensions that comprise the MOOSE flavored markdown.
  """

  def __init__(self, **kwargs):

    # Determine the root directory via git
    root = subprocess.check_output(['git', 'rev-parse', '--show-toplevel'], stderr=subprocess.STDOUT).strip('\n')

    # Define the configuration options
    self.config = dict()
    self.config['root']         = [root, "The root directory of the repository, if not provided the root is found using git."]
    self.config['make']         = [root, "The location of the Makefile responsible for building the application."]
    self.config['executable']   = ['', "The executable to utilize for generating application syntax."]
    self.config['locations']    = [dict(), "The locations to parse for syntax."]
    self.config['repo']         = ['', "The remote repository to create hyperlinks."]
    self.config['links']        = [dict(), "The set of paths for generating input file and source code links to objects."]
    self.config['docs_dir']     = ['docs', "The location of the documentation directory."]
    self.config['markdown_dir'] = [os.path.join('docs', 'content'), "The location of the markdown to be used for generating the site."]
    self.config['slides']       = [False, "Enable the parsing for creating reveal.js slides."]
    self.config['package']      = [False, "Enable the use of the MoosePackageParser."]
    self.config['graphviz']     = ['/opt/moose/graphviz/bin', 'The location of graphviz executable for use with diagrams.']
    self.config['dot_ext']      = ['svg', "The graphviz/dot output file extension (default: svg)."]
    self.config['pages']        = ['pages.yml', "The the pages file defining the site map."]
    self.config['hide']         = [[], "A list of input file syntax to hide from system."]

    # Construct the extension object
    super(MooseMarkdown, self).__init__(**kwargs)

  def extendMarkdown(self, md, md_globals):
    """
    Builds the extensions for MOOSE flavored markdown.
    """

    # Strip description from config
    config = self.getConfigs()

    # Generate YAML data from application
    exe = config['executable']
    if not os.path.exists(exe):
      log.error('The executable does not exist: {}'.format(exe))
    else:
      log.debug("Executing {} to extract syntax.".format(exe))
      raw = utils.runExe(exe, '--yaml')
      try:
        exe_yaml = utils.MooseYaml(raw)
      except:
        log.error('Failed to read YAML file, MOOSE and modules are likely not compiled correctly.')

    # Populate the database for input file and children objects
    input_files = collections.OrderedDict()
    child_objects = collections.OrderedDict()
    log.info('Building input and inheritance databases...')
    for key, path in config['links'].iteritems():
      input_files[key] =  MooseDocs.database.Database('.i', path, MooseDocs.database.items.InputFileItem, repo=config['repo'])
      child_objects[key] = MooseDocs.database.Database('.h', path, MooseDocs.database.items.ChildClassItem, repo=config['repo'])

    # Populate the syntax
    syntax = dict()
    for key, value in config['locations'].iteritems():
      if 'hide' in value:
        value['hide'] += config['hide']
      else:
        value['hide'] = config['hide']
      syntax[key] = MooseDocs.MooseApplicationSyntax(exe_yaml, **value)

    # Populate the pages yaml
    pages = []
    yml = MooseDocs.yaml_load(config['pages'])
    for match in re.finditer(r':(.*?\.md)', yaml.dump(yml, default_flow_style=False)):
      pages.append(match.group(1).strip())

    # Preprocessors
    bibtex = MooseBibtex(markdown_instance=md, docs_dir=config['docs_dir'], root=config['root'])
    md.preprocessors.add('moose_bibtex', bibtex , '_end')

    links = MooseMarkdownLinkPreprocessor(markdown_instance=md, pages=pages)
    md.preprocessors.add('moose_auto_link', links, '_begin')

    if config['slides']:
      slides = MooseSlidePreprocessor(markdown_instance=md)
      md.preprocessors.add('moose_slides', slides, '_end')

    # Block processors
    diagrams = MooseDiagram(md.parser, root=config['root'], docs_dir=config['docs_dir'], graphviz=config['graphviz'], ext=config['dot_ext'])
    md.parser.blockprocessors.add('diagrams', diagrams, '_begin')

    carousel = MooseSlider(md.parser, root=config['root'], docs_dir=config['docs_dir'])
    md.parser.blockprocessors.add('slideshow', carousel, '_begin')

    css = MooseCSS(md.parser, root=config['root'], docs_dir=config['docs_dir'])
    md.parser.blockprocessors.add('css', css, '_begin')

    # Inline Patterns
    object_markdown = MooseObjectSyntax(markdown_instance=md,
                                        yaml=exe_yaml,
                                        syntax=syntax,
                                        input_files=input_files,
                                        child_objects=child_objects,
                                        repo=config['repo'],
                                        root=config['root'],
                                        docs_dir=config['docs_dir'])
    md.inlinePatterns.add('moose_object_syntax', object_markdown, '_begin')

    system_markdown = MooseSystemSyntax(markdown_instance=md,
                                        yaml=exe_yaml,
                                        syntax=syntax,
                                        root=config['root'],
                                        docs_dir=config['docs_dir'])
    md.inlinePatterns.add('moose_system_syntax', system_markdown, '_begin')

    moose_input = MooseInputBlock(markdown_instance=md, root=config['root'], docs_dir=config['docs_dir'], repo=config['repo'])
    md.inlinePatterns.add('moose_input_block', moose_input , '<image_link')

    moose_cpp = MooseCppMethod(markdown_instance=md, root=config['root'], docs_dir=config['docs_dir'], make=config['make'], repo=config['repo'])
    md.inlinePatterns.add('moose_cpp_method', moose_cpp, '<image_link')

    moose_text = MooseTextFile(markdown_instance=md, root=config['root'], docs_dir=config['docs_dir'], repo=config['repo'])
    md.inlinePatterns.add('moose_text', moose_text, '<image_link')

    moose_image = MooseImageFile(markdown_instance=md, root=config['root'], docs_dir=config['docs_dir'])
    md.inlinePatterns.add('moose_image', moose_image, '<image_link')

    moose_status = MooseBuildStatus(markdown_instance=md, root=config['root'], docs_dir=config['docs_dir'])
    md.inlinePatterns.add('moose_build_status', moose_status, '_begin')

    if config['package']:
      package = MoosePackageParser(markdown_instance=md, root=config['root'], docs_dir=config['docs_dir'])
      md.inlinePatterns.add('moose_package_parser', package, '_end')

def makeExtension(*args, **kwargs):
  return MooseMarkdown(*args, **kwargs)
