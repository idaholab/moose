import os
import MooseDocs
import utils
import logging
log = logging.getLogger(__name__)

def generate_options(parser, subparser):
  """
  Command-line options for generate command.
  """

  generate_parser = subparser.add_parser('generate', help="Check that documentation exists for your application and generate the markdown documentation from MOOSE application executable.")
  generate_parser.add_argument('--disable-stubs', dest='stubs', action='store_false', help="Disable the creation of system and object stub markdown files.")
  generate_parser.add_argument('--disable-pages-stubs', dest='pages_stubs', action='store_false', help="Disable the creation the pages.yml files.")

  return generate_parser

def generate(config_file='moosedocs.yml', pages='pages.yml', stubs=False, pages_stubs=False, **kwargs):
  """
  Generates MOOSE system and object markdown files from the source code.

  Args:
    config_file[str]: (Default: 'moosedocs.yml') The MooseMkDocs project configuration file.
  """

  # Configuration file
  if not os.path.exists(config_file):
    raise IOError("The supplied configuration file was not found: {}".format(config_file))

  # Read the configuration
  config = MooseDocs.yaml_load(config_file)
  config = config['markdown_extensions'][-1]['MooseDocs.extensions.MooseMarkdown']

  # Run the executable
  exe = config['executable']
  if not os.path.exists(exe):
    log.error('The executable does not exist: {}'.format(exe))
  else:
    log.debug("Executing {} to extract syntax.".format(exe))
    raw = utils.runExe(exe, '--yaml')
    yaml = utils.MooseYaml(raw)

  # Populate the syntax
  for key, value in config['locations'].iteritems():
    if 'hide' in value:
      value['hide'] += config.get('hide', [])
    else:
      value['hide'] = config.get('hide', [])
    syntax = MooseDocs.MooseApplicationSyntax(yaml, name=key, stubs=stubs,  pages_stubs=pages_stubs, pages=pages, **value)
    log.info("Checking documentation for '{}'.".format(key))
    syntax.check()
