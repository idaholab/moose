#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import datetime
import mooseutils
from ..base import components
from ..common import exceptions
from ..tree import tokens, html
from . import command, core

def make_extension(**kwargs):
    return GitUtilsExtension(**kwargs)

class GitUtilsExtension(command.CommandExtension):
    """
    Adds ability to include git repository information.
    """

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        return config

    def extend(self, reader, renderer):
        self.requires(core, command)
        self.addCommand(reader, CommitCommand())

class CommitCommand(command.CommandComponent):
    COMMAND = 'git'
    SUBCOMMAND = 'commit'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        return settings

    def createToken(self, parent, info, page):
        content = info['inline'] if 'inline' in info else info['block']
        if content:
            raise exceptions.MooseDocsException("Content is not supported for the 'git commit' command.")

        if not mooseutils.is_git_repo():
            raise exceptions.MooseDocsException("The current working directory is not a git repository.")

        core.Word(parent, content=mooseutils.git_commit())
        return parent
