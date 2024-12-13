#!/usr/bin/env python3
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
import logging
import subprocess
import traceback
import mooseutils
import moosetree
import moosesyntax
from mooseutils.yaml_load import yaml_load
from .check_syntax import check_syntax
from .SQAReport import SQAReport
from .LogHelper import LogHelper

class SQAMooseAppReport(SQAReport):
    """
    Report of MooseObject and MOOSE syntax markdown pages.
    """
    def __init__(self, **kwargs):

        self.app_syntax = kwargs.pop('app_syntax', None)
        self.app_types = kwargs.pop('app_types', None)
        self.working_dir = kwargs.pop('working_dir', mooseutils.git_root_dir())
        self.exe_directory = kwargs.pop('exe_directory', mooseutils.git_root_dir())
        self.exe_name = kwargs.pop('exe_name', os.path.basename(self.exe_directory))
        self.content_directory = kwargs.pop('content_directory', os.path.join(self.exe_directory, 'doc', 'content'))
        self.object_prefix = kwargs.pop('object_prefix', os.path.join(self.content_directory, 'source'))
        self.syntax_prefix = kwargs.pop('syntax_prefix', os.path.join(self.content_directory, 'syntax'))
        self.remove = kwargs.pop('remove', None)
        self.alias = kwargs.pop('alias', None)
        self.unregister = kwargs.pop('unregister', None)
        self.allow_test_objects = kwargs.pop('allow_test_objects', False)
        super().__init__(**kwargs)

    def execute(self, **kwargs):
        """Perform app syntax checking"""

        # Check that the supplied content dir exists
        content_dir = mooseutils.eval_path(self.content_directory)
        if not os.path.isdir(content_dir):
            content_dir = os.path.join(self.working_dir, content_dir)
        if not os.path.isdir(content_dir):
            raise NotADirectoryError("'content_directory' input is not a directory: {}".format(content_dir))

        # Populate the available list of files
        file_cache = mooseutils.git_ls_files(content_dir)

        # Check that the supplied exe dir exists
        exe_dir = mooseutils.eval_path(self.exe_directory)
        if not os.path.isdir(exe_dir):
            exe_dir = os.path.join(self.working_dir, exe_dir)
        if not os.path.isdir(exe_dir):
            raise NotADirectoryError("'exe_directory' input is not a directory: {}".format(exe_dir))

        # Locate the executable
        exe = mooseutils.find_moose_executable(exe_dir, name=self.exe_name, show_error=False)
        if exe is None:
            raise OSError("An executable was not found in '{}' with a name '{}'.".format(exe_dir, self.exe_name))

        # Determine the application type (e.g., MooseTestApp)
        if self.app_types is None:
            out = subprocess.check_output([exe, '--show-type'], encoding='utf-8')
            match = re.search(r'^MooseApp Type:\s+(?P<type>.*?)$', out, flags=re.MULTILINE)
            if match:
                self.app_types = [match.group("type").replace('TestApp', 'App')]

        # Build syntax tree if not provided
        if self.app_syntax is None:

            # Get the removed/alias information
            remove = self._loadYamlFiles(self.remove)
            alias = self._loadYamlFiles(self.alias)
            unregister = self._loadYamlFiles(self.unregister)

            # Build the complete syntax tree
            self.app_syntax = moosesyntax.get_moose_syntax_tree(exe, remove=remove,
                                                                alias=alias, unregister=unregister)

        # Perform the checks
        kwargs.setdefault('syntax_prefix', mooseutils.eval_path(self.syntax_prefix))
        kwargs.setdefault('object_prefix', mooseutils.eval_path(self.object_prefix))
        kwargs.setdefault('allow_test_objects', self.allow_test_objects)
        logger = check_syntax(self.app_syntax, self.app_types, file_cache, **kwargs)

        return logger

    def _loadYamlFiles(self, filenames):
        """Load the removed/alias yml files"""
        content = dict()
        if filenames is not None:
            for fname in filenames:
                yml_file = mooseutils.eval_path(fname)
                if not os.path.isdir(yml_file):
                    yml_file = os.path.join(self.working_dir, yml_file)

                if not os.path.isfile(yml_file):
                    raise NotADirectoryError("YAML file is not a directory: {}".format(fname))

                content.update({fname:yaml_load(yml_file)})
        return content
