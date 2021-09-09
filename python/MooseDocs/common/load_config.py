#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""Tool for loading MooseDocs config hit file."""
import types
import importlib
import logging
import collections
from mooseutils import recursive_update
from mooseutils.yaml_load import yaml_load

import MooseDocs
from ..tree import pages
from ..common import exceptions

LOG = logging.getLogger(__name__)

# Set of extensions to load by default
DEFAULT_EXTENSIONS = ['MooseDocs.extensions.core',
                      'MooseDocs.extensions.shortcut',
                      'MooseDocs.extensions.floats',
                      'MooseDocs.extensions.command',
                      'MooseDocs.extensions.include',
                      'MooseDocs.extensions.style',
                      'MooseDocs.extensions.media',
                      'MooseDocs.extensions.listing',
                      'MooseDocs.extensions.table',
                      'MooseDocs.extensions.autolink',
                      'MooseDocs.extensions.devel',
                      'MooseDocs.extensions.package',
                      'MooseDocs.extensions.alert',
                      'MooseDocs.extensions.katex',
                      'MooseDocs.extensions.appsyntax',
                      'MooseDocs.extensions.bibtex',
                      'MooseDocs.extensions.common',
                      'MooseDocs.extensions.layout',
                      'MooseDocs.extensions.config',
                      'MooseDocs.extensions.materialicon',
                      'MooseDocs.extensions.acronym',
                      'MooseDocs.extensions.content',
                      'MooseDocs.extensions.graph',
                      'MooseDocs.extensions.heading',
                      'MooseDocs.extensions.gallery',
                      'MooseDocs.extensions.navigation',
                      'MooseDocs.extensions.template',
                      'MooseDocs.extensions.comment',
                      'MooseDocs.extensions.special',
                      'MooseDocs.extensions.ifelse',
                      'MooseDocs.extensions.pysyntax',
                      'MooseDocs.extensions.modal',
                      'MooseDocs.extensions.datetime',
                      'MooseDocs.extensions.gitutils']

DEFAULT_READER = 'MooseDocs.base.MarkdownReader'
DEFAULT_RENDERER = 'MooseDocs.base.MarkdownReader'
DEFAULT_TRANSLATOR = 'MooseDocs.base.Translator'
DEFAULT_EXECUTIONER = 'MooseDocs.base.ParallelQueue'

def load_config(filename, **kwargs):
    """
    Read the config.yml file and create the Translator object.
    """
    config = yaml_load(filename, root=MooseDocs.ROOT_DIR)

    # Replace 'default' and 'disable' key in Extensions to allow for recursive_update to accept command line
    for key in config.get('Extensions', dict()).keys():
        if config['Extensions'][key] == 'default':
            config['Extensions'][key] = dict()
        if config['Extensions'][key] == 'disable':
            config['Extensions'][key] = dict(active=False)

    # Apply command-line key value pairs
    recursive_update(config, kwargs)

    extensions = _yaml_load_extensions(config)
    reader = _yaml_load_object('Reader', config, DEFAULT_READER)
    renderer = _yaml_load_object('Renderer', config, DEFAULT_RENDERER)
    content = _yaml_load_content(config, reader.EXTENSIONS)
    executioner = _yaml_load_object('Executioner', config, DEFAULT_EXECUTIONER)
    translator = _yaml_load_object('Translator', config, DEFAULT_TRANSLATOR,
                                   content, reader, renderer, extensions, executioner)

    return translator, config

def load_configs(filenames, **kwargs):
    """
    Read the YAML files listed in filenames and create unique Translator objects for each. Each
    configuration should specify the same translator destination unless it is set globally via the
    kwargs. The kwargs are applied the same to all configurations (see the load_config() method).

    The content specified by each configuration is added to a common pool of content and then
    distributed to each Translator object. The contents output is a list of lists containing the
    pages that a corresponding translator is responsible for. Each translator creates independent
    Extensions, Reader, Renderer, and Executioner objects to build its designated page objects, but
    they still should have access to those built by any other translator.

    The local names of all page objects must be unique within the global content pool. The only
    exceptions are pages.Directory objects, for which duplicates may occur. Directories will simply
    be written out by the first translator that encounters them.
    """
    destination = kwargs.get('Translator', dict()).get('destination')
    destined = False
    translators = list()
    configurations = list()
    for file in filenames:
        trans, config = load_config(file, **kwargs)

        # Make sure translators are all outputting to the same destination
        current = config.get('Translator', dict()).get('destination')
        if current is not None:
            if destined and current != destination:
                msg = "The translator destination '{}' was specified by {}, but another " \
                      "configuration file used '{}'. Please specify a value in only one file or " \
                      "the same value in all files. Otherwise, use the kwargs to override all."
                raise exceptions.MooseDocsException(msg, current, file, destination)
            elif destination is None:
                destination = current
                destined = True

        translators.append(trans)
        configurations.append(config)

    if destined:
        for translator in translators:
            translator.update(destination=destination)

    # Set contents for each translator then loop through and distribute their contents to all others
    contents = [[page for page in translator.getPages()] for translator in translators]
    pooled = list() # initialize global content pool
    for index, translator in enumerate(translators):
        cotranslators = [t for t in translators if t is not translator]
        for page in contents[index]:
            if page.local in pooled:
                if not isinstance(page, pages.Directory):
                    msg = "A page or file '{}' was specified by {}, but one by the same name had " \
                          "already been added to the content pool by another configuration file."
                    raise exceptions.MooseDocsException(msg, page.local, filenames[index])
            else:
                pooled.append(page.local)
                for ct in cotranslators:
                    ct.addPage(page)

    return translators, contents, configurations

def load_extensions(ext_list, ext_configs=None):
    """
    Convert the supplied list into MooseDocs extension objects by calling the make_extension method.

    Inputs:
        ext_list[list]: List of extension modules or module names.
        ext_configs[dict]: A dict() connecting configurations to the module, the key is the
                           complete module name.
    """
    if ext_configs is None:
        ext_configs = dict()

    extensions = []
    for ext in ext_list:
        name, mod = _get_module(ext)
        if not hasattr(mod, 'make_extension'):
            msg = "The supplied module {} does not contain the required 'make_extension' function."
            raise exceptions.MooseDocsException(msg, name)
        else:
            obj = mod.make_extension(**ext_configs.get(name, dict()))
            # hack to allow build to disable via command line
            extensions.append(obj)

    return extensions

def _get_module(ext):
    """Helper for loading a module."""

    if isinstance(ext, types.ModuleType):
        name = ext.__name__
    elif isinstance(ext, str):
        name = ext
        try:
            ext = importlib.import_module(name)
        except ImportError as e:
            msg = "Failed to import the supplied '{}' module.\n{}"
            raise exceptions.MooseDocsException(msg, name, e)
    else:
        msg = "The supplied module ({}) must be a module type or a string, but a {} object "\
              "was provided."
        raise exceptions.MooseDocsException(msg, ext, type(ext))

    return name, ext

def _yaml_load_extensions(config):
    """Load extensions from the Extensions block of the YAML configuration file."""

    # Extensions block
    options = config.get('Extensions', dict())

    # Load default configuration
    ext_configs = collections.OrderedDict()
    disable_defaults = options.pop('disable_defaults', False)
    if not disable_defaults:
        for ext in DEFAULT_EXTENSIONS:
            ext_configs[ext] = dict()

    # Get configuration items from configuration
    for ext_type, settings in options.items():
        if (settings is not None) and ('type' in settings):
            msg = "Using 'type' for the extensions is deprecated, the type should be supplied " \
                  "as the key to the dictionary, rather than an arbitrary name."
            LOG.warning(msg)
            ext_type = settings.pop('type')

        if ext_type not in ext_configs:
            ext_configs[ext_type] = dict()

        if isinstance(settings, dict):
            ext_configs[ext_type].update(settings)

        else:
            msg = "The supplied settings for the '%s' extension must be dict() or the 'default' " \
                  "keyword should be used."
            LOG.error(msg, ext_type)

    return load_extensions(list(ext_configs.keys()), ext_configs)

def _yaml_load_object(name, config, default, *args):
    """Helper for loading MooseDocs objects: Reader, Renderer, Translator"""

    options = config.get(name, dict())
    obj_type = options.pop('type', default)
    try:
        return eval(obj_type)(*args, **options)
    except NameError:
        msg = "ERROR: The %s block must contain a valid object name."
        LOG.error(msg, name)

def _yaml_load_content(config, in_ext):
    """Load the 'Content' section."""
    options = config.get('Content', None)
    if options is None:
        msg = "The 'Content' section is required."
        raise exceptions.MooseDocsException(msg)

    items = MooseDocs.common.get_items(options)
    return MooseDocs.common.get_content(items, in_ext)
