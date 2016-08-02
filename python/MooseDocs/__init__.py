import extensions
import database
import commands

import utils
import yaml

from MarkdownTable import MarkdownTable
from MooseObjectParameterTable import MooseObjectParameterTable
from MooseObjectInformation import MooseObjectInformation
from MooseSystemInformation import MooseSystemInformation
from MooseApplicationSyntax import MooseApplicationSyntax
from MooseApplicationDocGenerator import MooseApplicationDocGenerator
from MooseSubApplicationDocGenerator import MooseSubApplicationDocGenerator

import os
MOOSE_REPOSITORY = 'https://github.com/idaholab/moose/blob/devel/'
MOOSE_DIR = os.getenv('MOOSE_DIR', os.path.join(os.getenv('HOME'), 'projects', 'moose'))
MOOSE_DOCS_DIR = os.path.join(MOOSE_DIR, 'docs', 'documentation')
MOOSE_DOXYGEN = 'http://mooseframework.com/docs/doxygen/moose/'

# Throw an exception if MOOSE_DIR is not found.
if not os.path.exists(MOOSE_DIR):
    raise Exception('The MOOSE directory was not located.')

import logging
logging.getLogger(__name__).addHandler(logging.NullHandler())

class MkMooseDocsFormatter(logging.Formatter):
    """
    A formater that is aware of the class hierachy of the MooseDocs library.

    Call the init_logging function to initialize the use of this custom fomatter.
    """
    COLOR = {'DEBUG':'CYAN', 'INFO':'RESET', 'WARNING':'YELLOW', 'ERROR':'RED', 'CRITICAL':'MAGENTA'}
    COUNTS = {'DEBUG':0, 'INFO':0, 'WARNING':0, 'ERROR':0, 'CRITICAL':0}

    def format(self, record):
        msg = logging.Formatter.format(self, record)

        if record.name.endswith('Item'):
            level = 4
        elif record.name.endswith('Database'):
            level = 3
        elif record.name.endswith('MooseInformationBase') or record.name.endswith('MooseObjectInformation') or record.name.endswith('MooseApplicationSyntax') or record.name.endswith('MooseSystemInformation'):
            level = 2
        elif record.name.endswith('MooseSubApplicationDocGenerator'):
            level = 1
        else:
            level = 0

        if record.levelname in ['DEBUG', 'WARNING', 'ERROR', 'CRITICAL']:
            msg = '{}{}: {}'.format(' '*4*level, record.levelname, msg)
        else:
            msg = '{}{}'.format(' '*4*level, msg)

        if record.levelname in self.COLOR:
            msg = utils.colorText(msg, self.COLOR[record.levelname])

        # Increment counts
        self.COUNTS[record.levelname] += 1

        return msg

def init_logging(verbose=False):
    """
    Call this function to initialize the MooseDocs logging formatter.
    """

    # Setup the logger object
    if verbose:
        level = logging.DEBUG
    else:
        level = logging.INFO

    # The markdown package dumps way too much information in debug mode (so always set it to INFO)
    log = logging.getLogger('MARKDOWN')
    log.setLevel(logging.INFO)

    # Setup the custom formatter
    log = logging.getLogger('MooseDocs')
    formatter = MkMooseDocsFormatter()
    handler = logging.StreamHandler()
    handler.setFormatter(formatter)
    log.addHandler(handler)
    log.setLevel(level)

    log = logging.getLogger('mkdocs')
    log.addHandler(handler)
    log.setLevel(level)

    return formatter

def yaml_load(filename, loader=yaml.Loader):
    """
    """

    def include(self, node):
        """
        Allow for the embedding of yaml files.
        http://stackoverflow.com/questions/528281/how-can-i-include-an-yaml-file-inside-another
        """
        filename = os.path.join(self._root, self.construct_scalar(node))
        if os.path.exists(filename):
            with open(filename, 'r') as f:
                return yaml.load(f, Loader)

    class Loader(loader):
        """
        """

        def __init__(self, stream):
            """
            Store the root directory for including other yaml files.
            """
            if isinstance(stream, file):
                self._root = os.path.split(stream.name)[0]
            else:
                self._root = os.getcwd()
            super(Loader, self).__init__(stream)

    ## Attach the include constructor to our custom loader.
    Loader.add_constructor('!include', include)

    with open(filename, 'r') as fid:
        yml = yaml.load(fid.read(), Loader)
    return yml
