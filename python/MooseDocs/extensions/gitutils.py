# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import subprocess

import mooseutils
import MooseDocs
from ..common import exceptions
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
        self.addCommand(reader, SubmoduleHashCommand())


class CommitCommand(command.CommandComponent):
    COMMAND = "git"
    SUBCOMMAND = "commit"

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        return settings

    def createToken(self, parent, info, page, settings):
        content = info["inline"] if "inline" in info else info["block"]
        if content:
            raise exceptions.MooseDocsException(
                "Content is not supported for the 'git commit' command."
            )

        if not mooseutils.git_is_repo():
            raise exceptions.MooseDocsException(
                "The current working directory is not a git repository."
            )

        core.Word(parent, content=mooseutils.git_commit())
        return parent


class SubmoduleHashCommand(command.CommandComponent):
    COMMAND = "git"
    SUBCOMMAND = "submodule-hash"

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings["url"] = (
            None,
            "If provided, prefix the hash with the url to create a link.",
        )
        settings["length"] = (
            None,
            "Length of the hash to output in order to shorten it.",
        )
        return settings

    def createToken(self, parent, info, page, settings):
        inline = "inline" in info
        if not inline:
            raise exceptions.MooseDocsException(
                "The '!git submodule-hash' command is an inline level command, use '[!git!submodule-hash](name)' instead."
            )

        name = info["inline"]

        # Search MOOSE_DIR and then ROOT_DIR (app directory)
        check_dirs = [MooseDocs.MOOSE_DIR]
        if MooseDocs.MOOSE_DIR != MooseDocs.ROOT_DIR:
            check_dirs.append(MooseDocs.ROOT_DIR)
        for check_dir in check_dirs:
            result = subprocess.run(
                ["git", "ls-tree", "HEAD", name],
                capture_output=True,
                text=True,
                check=True,
                cwd=check_dir,
            )

            if result.stdout:
                hash = result.stdout.split()[2]
                assert len(hash) == 40

                display_hash = hash
                if (length := settings["length"]) is not None:
                    length = int(length)
                    assert length <= 40 and length > 1
                    display_hash = hash[:length]

                url = settings["url"]
                if url is None:
                    core.Word(parent, content=display_hash)
                else:
                    core.Link(
                        parent,
                        url=f"{url.rstrip('/')}/{hash}",
                        string=display_hash,
                    )
                return parent

        raise exceptions.MooseDocsException(
            f"The submodule '{name}' was not located in {', '.join(check_dirs)}"
        )
