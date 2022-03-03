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

from mooseutils.yaml_load import yaml_load

import MooseDocs
from ..common import exceptions
from ..tree import tokens
from . import core, command

def make_extension(**kwargs):
    return PackageExtension(**kwargs)

class PackageExtension(command.CommandExtension):
    """
    Adds ability to link to MOOSE enviornment packages.
    """

    @staticmethod
    def defaultConfig():
        packages_config = yaml_load(os.path.join(MooseDocs.MOOSE_DIR,
                                                 'framework',
                                                 'doc',
                                                 'packages_config.yml'))

        config = command.CommandExtension.defaultConfig()

        # Assign a key/value for every item in packages_config.yml
        for k, v in packages_config.items():
            config[k] = (v, 'Default version for %s' % (k))

        return config

    def extend(self, reader, renderer):
        self.requires(core, command)
        self.addCommand(reader, PackageCodeReplace())
        self.addCommand(reader, PackageTextReplace())

class PackageCodeReplace(command.CommandComponent):
    """
    Code block replace __PACKAGE_NAME__ with a corresponding version, as
    specified in configuration file. You can specify the type of code block
    by providing the language=value command.

    The default language (if not provided) is bash: language=bash

    YAML Syntax:
        gcc: 7.3.0

    Markdown Syntax:
        !package! code
        /path/to/gcc-__GCC__
    """

    COMMAND = 'package'
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
        content = re.sub(r'__(?P<package>[A-Z][A-Z_]+)__', self._subFunction, content,
                         flags=re.UNICODE)
        core.Code(parent, style="max-height:{};".format(settings['max-height']),
                  language=settings['language'], content=content)
        return parent

    def _subFunction(self, match):
        version = self.extension.get(match.group('package').lower(), None)
        if version is not None:
            return str(version)
        return match.group(0)

class PackageTextReplace(command.CommandComponent):
    """
    In-line package name replacement with a corresponding version, as
    specified in the configuration file.

    YAML Syntax:
        gcc: 7.3.0

    Markdown Syntax:
        This is a sentence with gcc-[!package!gcc]

    yields:
        "This is a sentence with gcc-7.3.0"
    """
    COMMAND = 'package'
    SUBCOMMAND = '*'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        return settings

    def createToken(self, parent, info, page, settings):
        content = self.extension.get(info['subcommand'], dict())
        tokens.String(parent, content=str(content))
        return parent
