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
@mooseutils.addProperty('hidden', ptype=list)
@mooseutils.addProperty('remove', ptype=list)
@mooseutils.addProperty('alias', ptype=list)
@mooseutils.addProperty('unregister', ptype=list)
@mooseutils.addProperty('allow_test_objects', ptype=bool, default=False)
@mooseutils.addProperty('generate_stubs', ptype=bool, default=False)
@mooseutils.addProperty('dump_syntax', ptype=bool, default=False)
class SQAMooseAppReport(SQAReport):
    """
    Report of MooseObject and MOOSE syntax markdown pages.
    """
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        # Default attributes
        self.exe_name = os.path.basename(self.exe_directory)
        self.working_dir = self.working_dir or mooseutils.git_root_dir()
        self.content_directory = self.content_directory or os.path.join(self.exe_directory, 'doc', 'content')
        self.object_prefix = self.object_prefix or os.path.join(self.content_directory, 'source')
        self.syntax_prefix = self.syntax_prefix or os.path.join(self.content_directory, 'syntax')

    def execute(self, **kwargs):
        """Perform app syntax checking"""

        # Populate the available list of files
        content_dir = os.path.join(self.working_dir, self.content_directory)
        file_cache = mooseutils.git_ls_files(content_dir)

        # Build syntax tree if not provided
        if self.app_syntax is None:

            # Get the hidden/removed/alias information
            hide = self._loadYamlFiles(self.hidden)
            remove = self._loadYamlFiles(self.remove)
            alias = self._loadYamlFiles(self.alias)
            unregister = self._loadYamlFiles(self.unregister)

            # Locate the executable
            location = os.path.join(self.working_dir, self.exe_directory)
            exe = mooseutils.find_moose_executable(location, name=self.exe_name, show_error=False)

            # Build the complete syntax tree
            self.app_syntax = moosesyntax.get_moose_syntax_tree(exe, hide=hide, remove=remove,
                                                                alias=alias, unregister=unregister,
                                                                allow_test_objects=self.allow_test_objects)

        # Determine the application type (e.g., MooseTestApp)
        if self.app_types is None:
            out = mooseutils.runExe(exe, ['--type'])
            match = re.search(r'^MooseApp Type:\s+(?P<type>.*?)$', out, flags=re.MULTILINE)
            if match:
                self.app_types = [match.group("type").replace('TestApp', 'App')]

        # Perform the checks
        kwargs.setdefault('syntax_prefix', self.syntax_prefix)
        kwargs.setdefault('object_prefix', self.object_prefix)
        logger = check_syntax(self.app_syntax, self.app_types, file_cache, **kwargs)

        # Create stub pages
        if self.generate_stubs:
            func = lambda n: (not n.removed) \
                             and ('_md_file' in n) \
                             and ((n['_md_file'] is None) or n['_is_stub']) \
                             and ((n.group in self.app_types) \
                                  or (n.groups() == set(self.app_types)))
            for node in moosetree.iterate(self.app_syntax, func):
                print("STUB: ", node.fullpath())
                self._createStubPage(node)

        # Dump
        if self.dump_syntax:
            print(self.app_syntax)

        return logger

    def _createStubPage(self, node):
        """Copy template content to expected document location."""

        # Determine the correct markdown filename
        filename = node['_md_path']
        if isinstance(node, moosesyntax.ObjectNodeBase):
            filename = os.path.join(self.working_dir, node['_md_path'])
        elif isinstance(node, moosesyntax.SyntaxNode):
            action = moosetree.find(node, lambda n: isinstance(n, moosesyntax.ActionNode))
            filename = os.path.join(self.working_dir,os.path.dirname(node['_md_path']), 'index.md')

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
        tname = os.path.join(os.path.dirname(__file__), '..', '..', 'framework', 'doc', 'content',
                                             'templates', 'stubs', tname)

        # Read template and apply node content
        with open(tname, 'r') as fid:
            content = fid.read()
        content = mooseutils.apply_template_arguments(content, name=node.name, syntax=node.fullpath())

        # Write the content to the desired destination
        self._writeFile(filename, content)

    def _loadYamlFiles(self, filenames):
        """Load the hidden/removed/alias yml files"""
        content = dict()
        if filenames is not None:
            for fname in filenames:
                yml_file = os.path.join(self.working_dir, fname)
                content.update({fname:yaml_load(yml_file)})
        return content

    @staticmethod
    def _writeFile(filename, content):
        """A helper function that is easy to mock in tests"""
        os.makedirs(os.path.dirname(filename), exist_ok=True)
        with open(filename, 'w') as fid:
            fid.write(content)
