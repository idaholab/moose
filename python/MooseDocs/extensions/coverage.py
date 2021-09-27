#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import configparser
import MooseDocs
from ..common import exceptions
from ..base import components, LatexRenderer, HTMLRenderer, MarkdownReader
from ..tree import tokens, html, latex
from . import command, table, floats

def make_extension(**kwargs):
    return CoverageExtension(**kwargs)

class CoverageExtension(command.CommandExtension):
    """
    Adds coverage boxes (note, tip, error, warning, and construction) to display important information.
    """

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['prefix'] = ('Table', "The caption prefix for generated tables (e.g., Tbl.).")
        config['coverage-file'] = ('.coverage', ("The name of the coverage file, defined relative "
                                                 "to the repository root directory."))
        return config

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)

        c_file = os.path.join(MooseDocs.ROOT_DIR, self.get('coverage-file'))
        if not os.path.isfile(c_file):
            raise exceptions.MooseDocsException("The coverage file '{}' does not exist.", c_file)

        self._coverage = configparser.ConfigParser()
        self._coverage.read(c_file)

    @property
    def coverage(self):
        """Return the parsed coverage config object."""
        return self._coverage

    def extend(self, reader, renderer):
        self.requires(command, table, floats)
        self.addCommand(reader, CoverageTableCommand())
        self.addCommand(reader, CoverageValueCommand())

class CoverageTableCommand(command.CommandComponent):
    COMMAND = 'coverage'
    SUBCOMMAND = 'table'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings.update(floats.caption_settings())
        return settings

    def createToken(self, parent, info, page):

        if 'inline' in info:
            raise common.exceptions.MooseDocsException("The 'coverage table' command is a block level command, use '!coverage table' instead.")

        # Create a set of available keys within each section
        options = set()
        for _, sec in self.extension.coverage.items():
            options.update(sec.keys())

        # Build the table rows
        rows = list()
        for name, sec in self.extension.coverage.items():
            rows.append([name] + [sec.get(k, fallback='') for k in options])

        # Create complete table
        flt = floats.create_float(parent, self.extension, self.reader, page, self.settings,
                                  token_type=table.TableFloat)
        token = table.builder(rows, headings=['Section'] + list(options))
        token.parent = flt

        return parent

class CoverageValueCommand(command.CommandComponent):
    COMMAND = 'coverage'
    SUBCOMMAND = 'value'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['section'] = (None, "The name of the config section.")
        settings['option'] = (None, "The name of the option within the section.")
        return settings

    def createToken(self, parent, info, page):

        sec = self.settings['section']
        opt = self.settings['option']

        if sec not in self.extension.coverage:
            raise exceptions.MooseDocsException("The '{}' section does not exist in coverage file.",
                                                sec)

        if opt not in self.extension.coverage[sec]:
            raise exceptions.MooseDocsException("The '{}' option does not exist in '{}' section of the coverage file.",
                                                opt, sec)

        tokens.String(parent, content=self.extension.coverage[sec][opt])
        return parent
