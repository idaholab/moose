#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import re
import sys
import glob
import logging
import yaml
import mooseutils
import MooseDocs
from MooseDocs.common import exceptions

LOG = logging.getLogger(__name__)

def command_line_options(subparser, parent):
    """Command line options for 'init' command."""
    parser = subparser.add_parser('init', parents=[parent], help="Initialize new repository to use MooseDocs.")

    parser.add_argument('--config', default='config.yml',
                        help="The configuration file.")

    init_subparser = parser.add_subparsers(dest='init_command', help="Initialization command(s).")

    sqa_parser = init_subparser.add_parser('sqa', help="Initialize SQA documentation.")
    sqa_parser.add_argument('--app', required=True, type=str, help="Name of application to use in SQA template files.")
    sqa_parser.add_argument('--category', default=None, type=str, help="Name of application to use in SQA configuration category name; if not provided it will be the lower case of the 'app' option.")

def _write_file(filename, content):
    """Helper for easy mock"""
    with open(filename, 'w') as fid:
        fid.write(content)

def init_sqa_docs(app, category):
    """Adds markdown files that load MOOSE SQA template files."""

    # Loop through MOOSE SQA template files
    for tname in glob.glob(os.path.join(MooseDocs.MOOSE_DIR, 'framework', 'doc', 'content', 'templates', 'sqa', 'app*.template')):
        template_name = os.path.basename(tname)

        # Application SQA document filename
        fname = '{}{}'.format(category, template_name[3:-9])
        if fname.endswith('index.md'):
            fname = 'index.md'

        filename = os.path.join(os.getcwd(), 'content', 'sqa', fname)

        # Create the directory, if needed
        os.makedirs(os.path.dirname(filename), exist_ok=True)

        # Write the content to the output file
        content = "!template load file=sqa/{} app={} category={}".format(template_name, app, category)
        _write_file(filename, content)

def init_sqa_config(app, category):
    """Creates default file to load from MooseDocs configuration."""
    template = os.path.join(os.path.dirname(__file__), 'sqa_app.yml.template')

    with open(template, 'r') as fid:
        content = fid.read()

    repo = mooseutils.git_repo(MooseDocs.ROOT_DIR)
    content = mooseutils.apply_template_arguments(content, repo=repo, category=category, app=app)

    filename = os.path.join(os.getcwd(), 'sqa_{}.yml'.format(category))
    _write_file(filename, content)

def init_sqa_report(app, category):
    """Creates default SQA report configuration file."""
    template = os.path.join(os.path.dirname(__file__), 'sqa_reports.yml.template')

    with open(template, 'r') as fid:
        content = fid.read()

    content = mooseutils.apply_template_arguments(content, category=category, app=app)
    filename = os.path.join(os.getcwd(), 'sqa_reports.yml')
    _write_file(filename, content)

def update_config_with_sqa(filename, app, category):
    """Modifies the config.yml to support SQA."""

    with open(filename, 'r') as fid:
        config = mooseutils.yaml_load(filename, include=False)

    if 'MooseDocs.extensions.sqa' not in config['Extensions']:
        config['Extensions']['MooseDocs.extensions.sqa'] = dict()

    if 'MooseDocs.extensions.template' not in config['Extensions']:
        config['Extensions']['MooseDocs.extensions.template'] = dict(active=True)

    sqa = config['Extensions']['MooseDocs.extensions.sqa']
    sqa['active'] = True

    relpath = os.path.join('${ROOT_DIR}', os.path.relpath(os.getcwd(), MooseDocs.ROOT_DIR))
    sqa['reports'] = mooseutils.IncludeYamlFile([os.path.join(relpath, 'sqa_reports.yml')],
                                                MooseDocs.ROOT_DIR, filename, include=False)
    sqa['categories'] = dict()
    sqa['categories']['framework'] = mooseutils.IncludeYamlFile(['${MOOSE_DIR}/framework/doc/sqa_framework.yml'],
                                                                MooseDocs.ROOT_DIR, filename, include=False)
    sqa['categories'][category] = mooseutils.IncludeYamlFile([os.path.join(relpath, 'sqa_{}.yml'.format(category))],
                                                             MooseDocs.ROOT_DIR, filename, include=False)
    mooseutils.yaml_write(filename, config)

def main(options):
    errno = 0

    LOG.warning("The 'init' command is designed to help get started. The resulting setup " \
                "is not guaranteed to work and modifications are expected.")

    if options.init_command == 'sqa':
        category = options.category or options.app.lower()
        init_sqa_config(options.app, category)
        init_sqa_docs(options.app, category)
        init_sqa_report(options.app, category)
        update_config_with_sqa(options.config, options.app, category)
    else:
        LOG.error("The 'init' command requires a sub-command, see --help.")
        errno = 1

    return errno
