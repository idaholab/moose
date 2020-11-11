#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""Developer tools for MooseDocs."""
import argparse
import os
import re
import collections
import logging

import MooseDocs
import moosesqa
import moosetree
import mooseutils
import moosesyntax

from .. import common
#from ..common import exceptions
#from ..tree import syntax
from ..extensions import template

LOG = logging.getLogger(__name__)

def command_line_options(subparser, parent):
    """Define the 'syntax' command."""
    parser = subparser.add_parser('syntax',
                                  parents=[parent],
                                  help="Tool for dumping application syntax to screen.")
    parser.add_argument('--config', type=str, default='sqa_reports.yml',
                        help="The YAML config file for performing SQA checks.")

def main(opt):
    """./moosedocs syntax"""

    # Setup logging
    logger = logging.getLogger('MooseDocs')
    logger.handlers = list()
    logger.addHandler(moosesqa.SilentRecordHandler())

    # Get the report objects for the applications
    _, _, app_reports = moosesqa.get_sqa_reports(opt.config, app_report=True, doc_report=False, req_report=False)

    # Loop through all reports and generate the stub pages
    for report in app_reports:
        report.getReport() # this is needed to generate the app syntax
        print(report.app_syntax)

    logger.handlers[0].clear() # don't report errors, that is the job for check command
    return 0
