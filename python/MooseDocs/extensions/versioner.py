# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import os
import re
import threading
import sys
from typing import Optional

import MooseDocs
from ..common import exceptions
from . import core, command

# Needed to load the Versioner
sys.path.append(os.path.join(MooseDocs.MOOSE_DIR, "scripts"))
from versioner import *


def make_extension(**kwargs):
    return VersionerExtension(**kwargs)


class VersionerExtension(command.CommandExtension):
    """
    Adds ability to link to MOOSE enviornment packages.
    """

    PACKAGES = None
    """Shared Versioner packages across all class instances."""

    PACKAGES_LOCK = threading.Lock()
    """Lock for PACKAGES across all class instances."""

    @classmethod
    def getPackages(cls):
        """
        Get the Versioner packages.

        Is thread safe and will build the packages from the
        Versioner on first call.
        """
        with cls.PACKAGES_LOCK:
            if cls.PACKAGES is None:
                cls.PACKAGES = Versioner().get_packages("HEAD")
            return cls.PACKAGES

    def extend(self, reader, renderer):
        self.requires(core, command)
        self.addCommand(reader, VersionerCodeReplace())
        self.addCommand(reader, VersionerVersionReplace())
        self.addCommand(reader, VersionerFullVersionReplace())
        self.addCommand(reader, VersionerCondaVersionReplace())

    def getVersion(self, package_name, versioner_keys, must_exist=True):
        """
        Helper for getting a version in the commands
        """
        package = self.getPackages().get(package_name)
        if package is None:
            if must_exist:
                raise exceptions.MooseDocsException(f'Unknown package "{package_name}"')
            return None
        value = package
        for key in versioner_keys:
            value = getattr(value, key)
        return value


class VersionerCodeReplace(command.CommandComponent):
    """
    Code block replace:
        - __VERSIONER_VERSION_[package]__
        - __VERSIONER_CONDA_VERSION_[package]__
    with the version version and versioner conda version, respectively.
    version from the Versioner script and

    Markdown Syntax:
        !versioner! code
        module load moose-dev-openmpi/__VERSIONER_VERSION_MOOSE-DEV__
        conda install moose-dev=__VERSIONER_CONDA_VERSION_MOOSE-DEV__
        ...
    """

    COMMAND = "versioner"
    SUBCOMMAND = "code"

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings["max-height"] = ("350px", "The default height for listing content.")
        settings["language"] = (
            "bash",
            "The language to use for highlighting, if not supplied "
            "it will be inferred from the extension (if possible).",
        )
        return settings

    def createToken(self, parent, info, page, settings):
        content = info["inline"] if "inline" in info else info["block"]

        def replace(content, prefix, versioner_keys):
            def sub_function(match):
                package_dashed = match.group(1).lower().replace("_", "-")
                return self.extension.getVersion(package_dashed, versioner_keys)

            return re.sub(
                r"__VERSIONER_" + re.escape(prefix) + r"_(?P<package>[A-Z][A-Z_]+)__",
                sub_function,
                content,
                flags=re.UNICODE,
            )

        content = replace(content, "VERSION", ["full_version"])
        content = replace(content, "CONDA_VERSION", ["conda", "install"])

        core.Code(
            parent,
            style="max-height:{};".format(settings["max-height"]),
            language=settings["language"],
            content=content,
        )
        return parent


class VersionerReplaceBase(command.CommandComponent):
    COMMAND = "versioner"

    VERSIONER_KEYS: Optional[list[str]] = None
    """
    Keys in the Versioner to use for creating the token.

    Set in derived classes.
    """

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings["package"] = (None, "The package to get the version of.")
        settings["url"] = (
            None,
            "If provided, prefix the version with the url to create a link.",
        )
        settings["prefix"] = (None, "Prefix to add to the version.")
        return settings

    def createToken(self, parent, info, page, settings):
        package = settings.get("package")
        if package is None:
            raise exceptions.MooseDocsException('Missing required option "package"')

        assert isinstance(self.VERSIONER_KEYS, list)
        assert all(isinstance(v, str) for v in self.VERSIONER_KEYS)
        version = str(self.extension.getVersion(package, self.VERSIONER_KEYS, True))

        if (prefix := settings["prefix"]) is not None:
            assert isinstance(prefix, str)
            version = f"{prefix}{version}"

        if (url := settings["url"]) is not None:
            assert isinstance(url, str)
            core.Link(
                parent,
                url=f"{url.rstrip('/')}/{version}",
                string=version,
            )
        else:
            core.Word(parent, content=version)

        return parent


class VersionerVersionReplace(VersionerReplaceBase):
    """
    In-line versioner version replacement.

    Markdown Syntax:
        This is a sentence with petsc version [!versioner!version package=petsc]

    yields:
        "This is a sentence with petsc version v1.1.1"
    """

    SUBCOMMAND = "version"
    VERSIONER_KEYS = ["version"]


class VersionerFullVersionReplace(VersionerReplaceBase):
    """
    In-line versioner version replacement.

    Markdown Syntax:
        This is a sentence with petsc version [!versioner!full-version package=petsc]

    yields:
        "This is a sentence with petsc version v1.1.1_0"
    """

    SUBCOMMAND = "full-version"
    VERSIONER_KEYS = ["full_version"]


class VersionerCondaVersionReplace(VersionerReplaceBase):
    """
    In-line versioner conda version replacement.

    Markdown Syntax:
        This is a sentence with moose-dev version [!versioner!conda-version package=moose-dev]

    yields:
        "This is a sentence with moose-dev version 2024.01.01"
    """

    SUBCOMMAND = "conda-version"
    VERSIONER_KEYS = ["conda", "install"]
