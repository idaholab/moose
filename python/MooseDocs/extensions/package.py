#pylint: disable=missing-docstring

import os

from mooseutils.yaml_load import yaml_load

import MooseDocs
from MooseDocs.common import exceptions
from MooseDocs.extensions import command
from MooseDocs.tree import tokens

def make_extension(**kwargs):
    return PackageExtension(**kwargs)

class PackageExtension(command.CommandExtension):
    """
    Adds ability to link to MOOSE enviornment packages.
    """

    @staticmethod
    def defaultConfig():
        packages = yaml_load(os.path.join(MooseDocs.MOOSE_DIR, 'framework', 'doc', 'packages.yml'))

        config = command.CommandExtension.defaultConfig()
        config['packages'] = (packages, "A dict of packages by name.")
        config['link'] = (r'http://www.mooseframework.org/moose_packages',
                          "Location of packages.")
        return config

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(PackageCommand())

class PackageCommand(command.CommandComponent):
    COMMAND = 'package'
    SUBCOMMAND = 'name'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['arch'] = (None, "The name of the OS package name to retrieve.")
        return settings

    def createToken(self, info, parent):
        arch = self.settings['arch']
        packages = self.extension.get('packages')

        if arch not in packages:
            msg = "The supplied value for the 'arch' settings, {}, was not found."
            raise exceptions.TokenizeException(msg, arch)

        href = os.path.join(self.extension.get('link'), packages[arch])
        tokens.Link(parent, url=unicode(href), string=unicode(packages[arch]))
        return parent
