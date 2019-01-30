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

import mooseutils

import MooseDocs
from MooseDocs import common
from MooseDocs.common import exceptions
from MooseDocs.extensions import core, command, floats

def make_extension(**kwargs):
    return ListingExtension(**kwargs)

class ListingExtension(command.CommandExtension):
    """
    Provides !listing command for including source code from files.
    """

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['prefix'] = (u'Listing', "The caption prefix (e.g., Fig.).")
        return config

    def extend(self, reader, renderer):
        self.requires(core, command, floats)
        self.addCommand(reader, LocalListingCommand())
        self.addCommand(reader, FileListingCommand())
        self.addCommand(reader, InputListingCommand())

class LocalListingCommand(command.CommandComponent):
    COMMAND = 'listing'
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings.update(floats.caption_settings())
        settings.update(common.extractContentSettings())
        settings['max-height'] = (u'350px', "The default height for listing content.")
        settings['language'] = (None, "The language to use for highlighting, if not supplied it " \
                                      "will be inferred from the extension (if possible).")
        return settings

    def createToken(self, parent, info, page):
        flt = floats.create_float(parent, self.extension, self.reader, page, self.settings)
        content = info['inline'] if 'inline' in info else info['block']
        core.Code(flt, style="max-height:{};".format(self.settings['max-height']),
                  language=self.settings['language'], content=content)
        return parent

class FileListingCommand(LocalListingCommand):
    COMMAND = 'listing'
    SUBCOMMAND = '*'

    @staticmethod
    def defaultSettings():
        settings = LocalListingCommand.defaultSettings()
        settings.update(common.extractContentSettings())
        settings['link'] = (True, "Include a link to the filename after the listing.")
        return settings

    def createToken(self, parent, info, page):
        """
        Build the tokens needed for displaying code listing.
        """

        filename = common.check_filenames(info['subcommand'])
        flt = floats.create_float(parent, self.extension, self.reader, page, self.settings)

        # Create code token
        lang = self.settings.get('language')
        content = self.extractContent(filename)
        lang = lang if lang else common.get_language(filename)
        core.Code(flt, style="max-height:{};".format(self.settings['max-height']),
                  content=content, language=lang)

        # Add bottom modal
        if self.settings['link']:
            rel_filename = os.path.relpath(filename, MooseDocs.ROOT_DIR)

            # Get the complete file
            content = common.read(filename)
            settings = common.get_settings_as_dict(common.extractContentSettings())
            settings['strip-header'] = False
            content, _ = common.extractContent(content, settings)

            # Create modal for display the files a popup
            code = core.Code(None, language=lang, content=content)
            link = floats.create_modal_link(flt,
                                            url=unicode(rel_filename),
                                            content=code,
                                            title=unicode(filename),
                                            string=u'({})'.format(rel_filename))
            link['data-tooltip'] = unicode(rel_filename)


        return parent

    def extractContent(self, filename):
        """
        Extract content to display in listing code box.
        """
        content = common.read(filename)
        content, _ = common.extractContent(content, self.settings)
        return content


class InputListingCommand(FileListingCommand):
    """
    Special listing command for MOOSE hit input files.
    """

    COMMAND = 'listing'
    SUBCOMMAND = 'i'

    @staticmethod
    def defaultSettings():
        settings = FileListingCommand.defaultSettings()
        settings['block'] = (None, 'Space separated list of input file block names to include.')
        return settings

    def extractContent(self, filename):
        """Extract the file contents for display."""
        content = common.read(filename)
        if self.settings['block']:
            content = self.extractInputBlocks(content, self.settings['block'])

        content, _ = common.extractContent(content, self.settings)
        return content

    @staticmethod
    def extractInputBlocks(filename, blocks):
        """Remove input file block(s)"""
        hit = mooseutils.hit_load(filename)
        out = []
        for block in blocks.split(' '):
            node = hit.find(block)
            if node is None:
                msg = "Unable to find block '{}' in {}."
                raise exceptions.MooseDocsException(msg, block, filename)
            out.append(unicode(node.render()))
        return '\n'.join(out)
