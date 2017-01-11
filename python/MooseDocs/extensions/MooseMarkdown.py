import os
import sys
import re
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
from MooseSlider import MooseSlider
from MooseDiagram import MooseDiagram
from MooseCSS import MooseCSS
from MooseSlidePreprocessor import MooseSlidePreprocessor
from MooseBuildStatus import MooseBuildStatus
from MooseBibtex import MooseBibtex
from MooseSystemList import MooseSystemList
import MooseDocs
import utils

class MooseMarkdown(markdown.Extension):
  """
  Extensions that comprise the MOOSE flavored markdown.
  """

  def __init__(self, **kwargs):

    # Storage for the MooseLinkDatabase object
    self.syntax = None

    # Define the configuration options
    self.config = dict()
    self.config['executable']   = ['', "The executable to utilize for generating application syntax."]
    self.config['locations']    = [dict(), "The locations to parse for syntax."]
    self.config['repo']         = ['', "The remote repository to create hyperlinks."]
    self.config['links']        = [dict(), "The set of paths for generating input file and source code links to objects."]
    self.config['slides']       = [False, "Enable the parsing for creating reveal.js slides."]
    self.config['package']      = [False, "Enable the use of the MoosePackageParser."]
    self.config['graphviz']     = ['/opt/moose/graphviz/bin', 'The location of graphviz executable for use with diagrams.']
    self.config['dot_ext']      = ['svg', "The graphviz/dot output file extension (default: svg)."]

    # Construct the extension object
    super(MooseMarkdown, self).__init__(**kwargs)

    # Create the absolute path to the executable
    self.setConfig('executable', MooseDocs.abspath(self.getConfig('executable')))

  def execute(self):
    """
    Execute the supplied MOOSE application and return the YAML.
    """

    exe = self.getConfig('executable')
    if not (exe or os.path.exists(exe)):
      log.critical('The executable does not exist: {}'.format(exe))
      raise Exception('Critical Error')

    else:
      log.debug("Executing {} to extract syntax.".format(exe))
      try:
        raw = utils.runExe(exe, '--yaml')
        return utils.MooseYaml(raw)
      except:
        log.critical('Failed to read YAML file, MOOSE and modules are likely not compiled correctly.')
        raise Exception('Critical Error')


  def extendMarkdown(self, md, md_globals):
    """
    Builds the extensions for MOOSE flavored markdown.
    """
    md.registerExtension(self)

    # Create a config object
    config = self.getConfigs()

    # Extract YAML
    exe_yaml = self.execute()

    # Generate YAML data from application
    # Populate the database for input file and children objects
    log.debug('Creating input file and source code use database.')
    database = MooseDocs.MooseLinkDatabase(**config)

    # Populate the syntax
    self.syntax = collections.OrderedDict()
    for item in config['locations']:
      key = item.keys()[0]
      options = item.values()[0]
      options.setdefault('name', key.replace('_', ' ').title())
      self.syntax[key] = MooseDocs.MooseApplicationSyntax(exe_yaml, **options)

    # Preprocessors
    md.preprocessors.add('moose_bibtex', MooseBibtex(markdown_instance=md, **config), '_end')
    if config['slides']:
      md.preprocessors.add('moose_slides', MooseSlidePreprocessor(markdown_instance=md), '_end')

    # Block processors
    md.parser.blockprocessors.add('diagrams', MooseDiagram(md.parser, **config), '_begin')
    md.parser.blockprocessors.add('slider', MooseSlider(md.parser, **config), '_begin')
    md.parser.blockprocessors.add('css', MooseCSS(md.parser, **config), '_begin')

    # Inline Patterns
    object_markdown = MooseObjectSyntax(markdown_instance=md, yaml=exe_yaml, syntax=self.syntax, database=database, **config)
    md.inlinePatterns.add('moose_object_syntax', object_markdown, '_begin')

    system_markdown = MooseSystemSyntax(markdown_instance=md, yaml=exe_yaml, syntax=self.syntax, **config)
    md.inlinePatterns.add('moose_system_syntax', system_markdown, '_begin')

    system_list = MooseSystemList(markdown_instance=md, yaml=exe_yaml, syntax=self.syntax, **config)
    md.inlinePatterns.add('moose_system_list', system_list, '_begin')

    md.inlinePatterns.add('moose_input_block', MooseInputBlock(markdown_instance=md, **config), '<image_link')
    md.inlinePatterns.add('moose_cpp_method', MooseCppMethod(markdown_instance=md, **config), '<image_link')
    md.inlinePatterns.add('moose_text', MooseTextFile(markdown_instance=md, **config), '<image_link')
    md.inlinePatterns.add('moose_image', MooseImageFile(markdown_instance=md, **config), '<image_link')
    md.inlinePatterns.add('moose_build_status', MooseBuildStatus(markdown_instance=md, **config), '_begin')
    if config['package']:
      md.inlinePatterns.add('moose_package_parser', MoosePackageParser(markdown_instance=md, **config), '_end')

def makeExtension(*args, **kwargs):
  return MooseMarkdown(*args, **kwargs)
