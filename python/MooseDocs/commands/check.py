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

from .. import common
#from ..common import exceptions
#from ..tree import syntax
from ..extensions import template

LOG = logging.getLogger(__name__)

def command_line_options(subparser, parent):
    """Define the 'check' command."""
    parser = subparser.add_parser('check',
                                  parents=[parent],
                                  help="Tool for performing SQA error checking and "
                                       "creating/updating documentation stub pages.")

    parser.add_argument('--config', type=str, default='sqa_reports.yml',
                        help="The YAML config file for performing SQA checks.")

    parser.add_argument('--reports', nargs='+', default=['doc', 'req', 'app'], choices=['doc', 'req', 'app'],
                        help='Select the reports to produce.')

    parser.add_argument('--show-warnings', action='store_true',
                        help='Display all report warnings.')

    parser.add_argument('--app-reports', nargs='+', default=None,
                        help='Limit to the following application reports (e.g. --app-reports navier_stokes')
    parser.add_argument('--req-reports', nargs='+', default=None,
                        help='Limit to the following requirement reports (e.g. --req-reports navier_stokes')

def _print_reports(title, reports, status):
    """Helper for printing SQAReport objects and propagating status"""
    if reports:
        print(mooseutils.colorText('\n{0}\n{1} REPORT(S):\n{0}\n'.format('-'*80, title.upper()), 'MAGENTA'), end='', flush=True)
        for report in reports:
            if report.status != 1 or report.show_warning:
                print(report.getReport(), '\n')
            status = report.status if status < report.status else status
    return status

def _enable_warnings(reports):
    """Helper for enabling all warnings"""
    if reports:
        for rep in reports:
            rep.show_warning = True

def main(opt):
    """./moosedocs check"""

    # Enable/disable different reports
    kwargs = {'app_report':'app' in opt.reports,
              'doc_report':'doc' in opt.reports,
              'req_report':'req' in opt.reports}

    # Change to a silent handler
    logger = logging.getLogger('MooseDocs')
    logger.handlers = list()
    logger.addHandler(moosesqa.SilentRecordHandler())

    # Create the SQAReport objects
    doc_reports, req_reports, app_reports = moosesqa.get_sqa_reports(opt.config, **kwargs)

    # Limit the reports
    if opt.app_reports and app_reports:
        app_reports = [report for report in app_reports if report.title in opt.app_reports]
    if opt.req_reports and req_reports:
        req_reports = [report for report in req_reports if report.title in opt.req_reports]

    # Apply 'show_warnings' option
    if opt.show_warnings:
        _enable_warnings(app_reports)
        _enable_warnings(doc_reports)
        _enable_warnings(req_reports)

    # Execute and display reports
    status = _print_reports('MooseApp', app_reports, 0)
    status = _print_reports('Document', doc_reports, status)
    status = _print_reports('Requirement', req_reports, status)
    return status > 1 # 0 - PASS; 1-WARNING; 2-ERROR (Only ERROR is a failure)
