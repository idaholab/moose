#pylint: disable=missing-docstring,attribute-defined-outside-init
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import mooseutils
from MooseDocs.extensions import command, core

def make_extension(**kwargs):
    return CivetExtension(**kwargs)

class CivetExtension(command.CommandExtension):
    "Adds ability to include CIVET links."""

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['owner'] = (None, "The owner of the repository, e.g. 'idaholab'.")
        config['repo'] = (None, "The repository name, e.g. 'moose'.")
        return config

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, CivetResultsCommand())

class CivetResultsCommand(command.CommandComponent):
    COMMAND = 'civet'
    SUBCOMMAND = 'results'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['owner'] = (None, "The owner of the repository, e.g. 'idaholab'.")
        settings['repo'] = (None, "The repository name, e.g. 'moose'.")
        return settings

    def createToken(self, parent, info, page):
        owner = self.settings['owner'] or self.extension.get('owner')
        repo = self.settings['repo'] or self.extension.get('repo')
        sha = mooseutils.git_commit()
        url = u'https://civet.inl.gov/sha_events/{}/{}/{}'.format(owner, repo, sha)
        return core.Link(parent, url=url)
