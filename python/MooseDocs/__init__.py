import extensions
import database
import utils

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
    COLOR = {'DEBUG':'GREEN', 'INFO':'RESET', 'WARNING':'YELLOW', 'ERROR':'RED', 'CRITICAL':'MAGENTA'}

    def format(self, record):
        msg = logging.Formatter.format(self, record)

        if record.name.endswith('Item'):
            level = 4
        elif record.name.endswith('Database'):
            level = 3
        elif record.name.endswith('MooseObjectInformation') or record.name.endswith('MooseApplicationSyntax') or record.name.endswith('MooseSystemInformation'):
            level = 2
        elif record.name.endswith('MooseSubApplicationDocGenerator'):
            level = 1
        else:
            level = 0

        if record.levelname in ['WARNING', 'ERROR', 'CRITICAL']:
            msg = '{}{}: {}'.format(' '*4*level, record.levelname, msg)
        else:
            msg = '{}{}'.format(' '*4*level, msg)

        if record.levelname in self.COLOR:
            msg = utils.colorText(msg, self.COLOR[record.levelname])

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
