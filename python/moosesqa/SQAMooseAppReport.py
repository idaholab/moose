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

@mooseutils.addProperty('app_syntax', ptype=moosesyntax.SyntaxNode)
@mooseutils.addProperty('app_types', ptype=list)
@mooseutils.addProperty('working_dir', ptype=str)
@mooseutils.addProperty('exe_name', ptype=str)
@mooseutils.addProperty('exe_directory', ptype=str)
@mooseutils.addProperty('content_directory', ptype=str)
@mooseutils.addProperty('object_prefix', ptype=str)
@mooseutils.addProperty('syntax_prefix', ptype=str)
@mooseutils.addProperty('remove', ptype=list)
@mooseutils.addProperty('alias', ptype=list)
@mooseutils.addProperty('unregister', ptype=list)
@mooseutils.addProperty('allow_test_objects', ptype=bool, default=False)
class SQAMooseAppReport(SQAReport):
    """
    Report of MooseObject and MOOSE syntax markdown pages.
    """
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        # Default attributes
        self.exe_directory = self.exe_directory or mooseutils.git_root_dir()
        self.exe_name = self.exe_name or os.path.basename(self.exe_directory)
        self.working_dir = self.working_dir or mooseutils.git_root_dir()
        self.content_directory = self.content_directory or os.path.join(self.exe_directory, 'doc', 'content')
        self.object_prefix = self.object_prefix or os.path.join(self.content_directory, 'source')
        self.syntax_prefix = self.syntax_prefix or os.path.join(self.content_directory, 'syntax')

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
            out = subprocess.check_output([exe, '--type'], encoding='utf-8')
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
