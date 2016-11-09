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

class Config(object):
  """
  A helper class for extracting a sub-set of configuration options from a dictionary.
  """
  def __init__(self, config):
    self.__config = config

  def __getitem__(self, key):
    """
    Retains dictionary-like access via [].

    Args:
      key[str]: The desired dictionary key to get from the configuration.
    """
    return self.__config[key]

  def __call__(self, keys):
    """
    Creates a sub-dictionary given the space separated keys.

    Args:
      keys[str]: Space separated keys to create a dictionary from (e.g., 'foo bar')
    """
    output = dict()
    for k in keys.split(' '):
      output[k] = self[k]
    return output


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
    self.config['slides']       = [False, "Enable the parsing for creating reveal.js slides."]
    self.config['package']      = [False, "Enable the use of the MoosePackageParser."]
    self.config['graphviz']     = ['/opt/moose/graphviz/bin', 'The location of graphviz executable for use with diagrams.']
    self.config['dot_ext']      = ['svg', "The graphviz/dot output file extension (default: svg)."]
    self.config['hide']         = [[], "A list of input file syntax to hide from system."]

    # Construct the extension object
    super(MooseMarkdown, self).__init__(**kwargs)




  def extendMarkdown(self, md, md_globals):
    """
    Builds the extensions for MOOSE flavored markdown.
    """

    # Create a config object
    config = Config(self.getConfigs())

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


    # Preprocessors
    bibtex = MooseBibtex(markdown_instance=md, **config('docs_dir root'))
    md.preprocessors.add('moose_bibtex', bibtex , '_end')

    if config['slides']:
      slides = MooseSlidePreprocessor(markdown_instance=md)
      md.preprocessors.add('moose_slides', slides, '_end')

    # Block processors
    diagrams = MooseDiagram(md.parser, **config('root docs_dir graphviz dot_ext'))
    md.parser.blockprocessors.add('diagrams', diagrams, '_begin')

    carousel = MooseSlider(md.parser, **config('root docs_dir'))
    md.parser.blockprocessors.add('slideshow', carousel, '_begin')

    css = MooseCSS(md.parser, **config('root docs_dir'))
    md.parser.blockprocessors.add('css', css, '_begin')

    # Inline Patterns
    object_markdown = MooseObjectSyntax(markdown_instance=md,
                                        yaml=exe_yaml,
                                        syntax=syntax,
                                        input_files=input_files,
                                        child_objects=child_objects,
                                        **config('repo root docs_dir'))
    md.inlinePatterns.add('moose_object_syntax', object_markdown, '_begin')

    system_markdown = MooseSystemSyntax(markdown_instance=md,
                                        yaml=exe_yaml,
                                        syntax=syntax,
                                        **config('root docs_dir'))
    md.inlinePatterns.add('moose_system_syntax', system_markdown, '_begin')

    moose_input = MooseInputBlock(markdown_instance=md, **config('root docs_dir repo'))
    md.inlinePatterns.add('moose_input_block', moose_input , '<image_link')

    moose_cpp = MooseCppMethod(markdown_instance=md, **config('root docs_dir repo make'))
    md.inlinePatterns.add('moose_cpp_method', moose_cpp, '<image_link')

    moose_text = MooseTextFile(markdown_instance=md, **config('root docs_dir repo'))
    md.inlinePatterns.add('moose_text', moose_text, '<image_link')

    moose_image = MooseImageFile(markdown_instance=md, **config('root docs_dir'))
    md.inlinePatterns.add('moose_image', moose_image, '<image_link')

    moose_status = MooseBuildStatus(markdown_instance=md, **config('root docs_dir'))
    md.inlinePatterns.add('moose_build_status', moose_status, '_begin')

    if config['package']:
      package = MoosePackageParser(markdown_instance=md, **config('root docs_dir'))
      md.inlinePatterns.add('moose_package_parser', package, '_end')

def makeExtension(*args, **kwargs):
  return MooseMarkdown(*args, **kwargs)
