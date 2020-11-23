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
from ..extensions import template

LOG = logging.getLogger(__name__)

def command_line_options(subparser, parent):
    """Define the 'generate' command."""
    parser = subparser.add_parser('generate',
                                  parents=[parent],
                                  help="Tool for creating/updating documentation stub pages.")
    parser.add_argument('app_types', nargs='+', type=str,
                        help="A list of app types to use when generate stub pages (e.g., StochasticToolsApp).")
    parser.add_argument('--config', type=str, default='sqa_reports.yml',
                        help="The YAML config file for performing SQA checks.")

def main(opt):
    """./moosedocs generate"""

    # Setup logging
    logger = logging.getLogger('MooseDocs')
    logger.handlers = list()
    logger.addHandler(moosesqa.SilentRecordHandler())

    # Get the report objects for the applications
    _, _, app_reports = moosesqa.get_sqa_reports(opt.config, app_report=True, doc_report=False, req_report=False)

    # Loop through all reports and generate the stub pages
    for report in app_reports:
        report.app_types = opt.app_types
        report.getReport() # this is needed to generate the app syntax
        for node in moosetree.iterate(report.app_syntax, lambda n: _shouldCreateStub(report, n)):
            _createStubPage(report, node)

    logger.handlers[0].clear() # don't report errors, that is the job for check command
    return 0

def _shouldCreateStub(report, n):
    return (not n.removed) \
        and ('_md_file' in n) \
        and ((n['_md_file'] is None) or n['_is_stub']) \
        and ((n.group in report.app_types) \
             or (n.groups() == set(report.app_types)))

def _createStubPage(report, node):
    """Copy template content to expected document location."""

    # Determine the correct markdown filename
    filename = node['_md_path']
    if isinstance(node, moosesyntax.ObjectNodeBase):
        filename = os.path.join(report.working_dir, node['_md_path'])
    elif isinstance(node, moosesyntax.SyntaxNode):
        action = moosetree.find(node, lambda n: isinstance(n, moosesyntax.ActionNode))
        filename = os.path.join(report.working_dir,os.path.dirname(node['_md_path']), 'index.md')

    # Determine the source template
    tname = None
    if isinstance(node, moosesyntax.SyntaxNode):
        tname = 'moose_system.md.template'
    elif isinstance(node, moosesyntax.MooseObjectNode):
        tname = 'moose_object.md.template'
    elif isinstance(node, moosesyntax.ActionNode):
        tname = 'moose_action.md.template'
    else:
        raise Exception("Unexpected syntax node type.")

    # Template file
    tname = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'framework',
                                         'doc', 'content','templates', 'stubs', tname))

    # Read template and apply node content
    with open(tname, 'r') as fid:
        content = fid.read()
    content = mooseutils.apply_template_arguments(content, name=node.name, syntax=node.fullpath())

    # Write the content to the desired destination
    print("Creating/updating stub page: {}".format(filename))
    _writeFile(filename, content)

def _writeFile(filename, content):
    """A helper function that is easy to mock in tests"""
    os.makedirs(os.path.dirname(filename), exist_ok=True)
    with open(filename, 'w') as fid:
        fid.write(content)
