#pylint: disable=missing-docstring
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
from MooseDocs.common import exceptions
from MooseDocs.extensions import core, command
from MooseDocs.tree import tokens

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
        for k, v in packages_config.iteritems():
            if k != 'moose_packages':
                config[k] = (v, 'Default version for %s' % (k))
            else:
                config[k] = (v, 'MOOSE Environment installer package')

        config['link'] = (r'http://www.mooseframework.org/moose_packages',
                          "Location of packages.")

        return config

    def extend(self, reader, renderer):
        self.requires(core, command)
        self.addCommand(reader, PackageCommand())
        self.addCommand(reader, PackageCodeReplace())
        self.addCommand(reader, PackageTextReplace())

class PackageCommand(command.CommandComponent):
    """
    Replace arch with matching moose-environment package, as specified in
    the yaml configuration file.

    YAML Syntax:
        moose_packages:
            centos: moose-environment-1_centos.rpm

    Markdown Syntax:
        !!package name arch=centos!!
    """
    COMMAND = 'package'
    SUBCOMMAND = 'name'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['arch'] = (None, "The name of the OS package name to retrieve.")
        return settings

    def createToken(self, parent, info, page):
        arch = self.settings['arch']
        packages = self.extension.get('moose_packages', dict())

        if arch not in packages:
            msg = "The supplied value for the 'arch' settings, {}, was not found."
            raise exceptions.MooseDocsException(msg, arch)

        href = os.path.join(self.extension.get('link'), packages[arch])
        core.Link(parent, url=unicode(href), string=unicode(packages[arch]))
        return parent

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
        settings['max-height'] = (u'350px', "The default height for listing content.")
        settings['language'] = (u'bash', "The language to use for highlighting, if not supplied " \
                                         "it will be inferred from the extension (if possible).")
        return settings

    def createToken(self, parent, info, page):
        content = info['inline'] if 'inline' in info else info['block']
        content = re.sub(r'__(?P<package>[A-Z][A-Z_]+)__', self.subFunction, content,
                         flags=re.UNICODE)
        core.Code(parent, style="max-height:{};".format(self.settings['max-height']),
                  language=self.settings['language'], content=content)
        return parent

    def subFunction(self, match):
        key = match.group('package')
        for package in self.extension.keys():
            if package.upper() == key:
                version = self.extension.get(package)
                return unicode(version)
        return match.group(0)

class PackageTextReplace(command.CommandComponent):
    """
    In-line package name replacement with a corresponding version, as
    specified in the configuration file.

    YAML Syntax:
        gcc: 7.3.0

    Markdown Syntax:
        This is a sentence with gcc-!!package gcc!!

    yields:
        "This is a sentence with gcc-7.3.0"
    """
    COMMAND = 'package'
    SUBCOMMAND = '*'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        return settings

    def createToken(self, parent, info, page):
        content = self.extension.get(info['subcommand'], dict())
        tokens.String(parent, content=unicode(content))
        return parent
