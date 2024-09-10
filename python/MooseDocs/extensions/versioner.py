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

import MooseDocs
from ..common import exceptions
from ..tree import tokens
from . import core, command

# Needed to load the Versioner
sys.path.append(os.path.join(MooseDocs.MOOSE_DIR, 'scripts'))
from versioner import *

def make_extension(**kwargs):
    return VersionerExtension(**kwargs)

class VersionerExtension(command.CommandExtension):
    """
    Adds ability to link to MOOSE enviornment packages.
    """

    def __init__(self, **kwargs):
      super().__init__(**kwargs)

      self.version_meta = Versioner().version_meta()

    def extend(self, reader, renderer):
        self.requires(core, command)
        self.addCommand(reader, VersionerCodeReplace())
        self.addCommand(reader, VersionerVersionReplace())
        self.addCommand(reader, VersionerCondaVersionReplace())

    def getVersion(self, package, versioner_keys, must_exist=True):
        """
        Helper for getting a version in the commands
        """
        version_meta = self.version_meta.get(package)
        if version_meta is None:
            if must_exist:
                raise exceptions.MooseDocsException(f'Unknown package "{package}"')
            return None
        value = version_meta
        for key in versioner_keys:
            value = value.get(key)
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

    COMMAND = 'versioner'
    SUBCOMMAND = 'code'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['max-height'] = ('350px', "The default height for listing content.")
        settings['language'] = ('bash', "The language to use for highlighting, if not supplied " \
                                         "it will be inferred from the extension (if possible).")
        return settings

    def createToken(self, parent, info, page, settings):
        content = info['inline'] if 'inline' in info else info['block']

        def replace(content, prefix, versioner_keys):
            def sub_function(match):
                package_dashed = match.group(1).lower().replace('_', '-')
                return self.extension.getVersion(package_dashed, versioner_keys)
            return re.sub(r'__VERSIONER_' + re.escape(prefix) + r'_(?P<package>[A-Z][A-Z_]+)__',
                          sub_function, content, flags=re.UNICODE)
        content = replace(content, 'VERSION', ['hash'])
        content = replace(content, 'CONDA_VERSION', ['conda', 'version_and_build'])

        core.Code(parent, style="max-height:{};".format(settings['max-height']),
                  language=settings['language'], content=content)
        return parent

class VersionerReplaceBase(command.CommandComponent):
    COMMAND = 'versioner'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['package'] = (None, "The package to get the version of")
        return settings

    def createTokenBase(self, parent, info, package, settings, versioner_keys):
        package = settings.get('package')
        if package is None:
            raise exceptions.MooseDocsException('Missing required option "package"')
        version = self.extension.getVersion(package, versioner_keys, True)
        tokens.String(parent, content=str(version))
        return parent

class VersionerVersionReplace(VersionerReplaceBase):
    """
    In-line versioner version replacement.

    Markdown Syntax:
        This is a sentence with moose-dev version [!versioner!version package=moose-dev]

    yields:
        "This is a sentence with moose-dev version abcd123"
    """
    SUBCOMMAND = 'version'

    def createToken(self, parent, info, page, settings):
        return self.createTokenBase(parent, info, page, settings, ['hash'])

class VersionerCondaVersionReplace(VersionerReplaceBase):
    """
    In-line versioner conda version replacement.

    Markdown Syntax:
        This is a sentence with moose-dev version [!versioner!conda_version package=moose-dev]

    yields:
        "This is a sentence with moose-dev version 2024.01.01"
    """
    SUBCOMMAND = 'conda_version'

    def createToken(self, parent, info, page, settings):
        return self.createTokenBase(parent, info, page, settings, ['conda', 'version_and_build'])
