#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################
#pylint: enable=missing-docstring

import os
import logging

import mooseutils

import MooseDocs
from .. import common

LOG = logging.getLogger(__name__)

def check_options(parser):
    """
    Command-line options for check command.
    """
    parser.add_argument('--config-file', type=str, default='website.yml',
                        help="The configuration file to use for building the documentation using "
                             "MOOSE. (Default: %(default)s)")
    parser.add_argument('--locations', nargs='+',
                        help="List of locations to consider, names should match the keys listed "
                             "in the configuration file.")
    parser.add_argument('--generate', action='store_true',
                        help="When checking the application for complete documentation generate "
                             "any missing markdown documentation files.")

def check(config_file=None, locations=None, generate=None):
    """
    Performs checks and optionally generates stub pages for missing documentation.
    """
    # Read the configuration
    app_ext = 'MooseDocs.extensions.app_syntax'
    config = MooseDocs.load_config(config_file)
    if app_ext not in config:
        mooseutils.MooseException("The 'check' utility requires the 'app_syntax' extension.")
    ext_config = config[app_ext]

    # Run the executable
    exe = MooseDocs.abspath(ext_config['executable'])
    if not os.path.exists(exe):
        raise IOError('The executable does not exist: {}'.format(exe))
    else:
        LOG.debug("Executing %s to extract syntax.", exe)
        raw = mooseutils.runExe(exe, '--yaml')
        yaml = mooseutils.MooseYaml(raw)

    # Populate the syntax
    for loc in ext_config['locations']:
        for key, value in loc.iteritems():
            if (locations is None) or (key in locations):
                value['group'] = key
                syntax = common.MooseApplicationSyntax(yaml, generate=generate,
                                                       install=ext_config['install'], **value)
                LOG.info("Checking documentation for '%s'.", key)
                syntax.check()

    return None
