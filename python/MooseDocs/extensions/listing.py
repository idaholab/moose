#pylint: disable=missing-docstring
import os
import re

import mooseutils

import MooseDocs
from MooseDocs import common
from MooseDocs.common import exceptions
from MooseDocs.extensions import command, floats
from MooseDocs.tree import tokens

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

        self.requires(command, floats)
        self.addCommand(LocalListingCommand())
        self.addCommand(FileListingCommand())
        self.addCommand(InputListingCommand())

class LocalListingCommand(command.CommandComponent):
    COMMAND = 'listing'
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['caption'] = (None, "The caption to use for the listing content.")
        settings['prefix'] = (None, "Text to include prior to the included text.")
        settings['max-height'] = (u'350px', "The default height for listing content.")
        settings['language'] = (None, "The language to use for highlighting, if not supplied it " \
                                      "will be inferred from the extension (if possible).")
        return settings

    def createToken(self, info, parent):
        flt = floats.Float(parent)
        content = info['inline'] if 'inline' in info else info['block']
        self.addCaption(flt)
        tokens.Code(flt, style="max-height:{};".format(self.settings['max-height']),
                    language=self.settings['language'], code=content)
        return parent

    # TODO: this is repeated in media, table, make a helper...(CaptionMixin)
    def addCaption(self, flt):

        cap = self.settings['caption']
        key = self.settings['id']
        if key:
            if self.settings['prefix'] is not None:
                prefix = self.settings['prefix']
            else:
                prefix = self.extension.get('prefix')
            caption = floats.Caption(flt, key=key, prefix=prefix)
            if cap:
                self.translator.reader.parse(caption, cap, MooseDocs.INLINE)
        elif cap:
            caption = floats.Caption(flt)
            self.translator.reader.parse(caption, cap, MooseDocs.INLINE)

class FileListingCommand(LocalListingCommand):
    COMMAND = 'listing'
    SUBCOMMAND = '*'

    @staticmethod
    def defaultSettings():
        settings = LocalListingCommand.defaultSettings()
        settings['caption'] = (None, "The caption to use for the listing content.")
        settings['link'] = (True, "Include a link to the filename after the listing.")
        settings['header'] = (None, "Text to include prior to the included text.")
        settings['footer'] = ('', "Text to include after to the included text.")
        settings['indent'] = (0, "The level of indenting to apply to the included text.")
        settings['strip-header'] = (True, "When True the MOOSE header is removed for display.")
        settings['strip-extra-newlines'] = (True, "Removes extraneous new lines from the text.")
        settings['strip-leading-whitespace'] = (False, "When True leading white-space is removed "
                                                       "from the included text.")
        settings['line'] = (None, "A portion of text that unique identifies a single line to "
                                  "include.")
        settings['re'] = (False, "Extract content via a regex, if the 'content' group exists it " \
                                 "is used as the desired content, otherwise group 0 is used.")
        settings['re-flags'] = ('re.M|re.S|re.U', "Python re flags.")
        settings['start'] = (None, "A portion of text that unique identifies the starting "
                                   "location for including text, if not provided the beginning "
                                   "of the file is utilized.")
        settings['end'] = (None, "A portion of text that unique identifies the ending location "
                                 "for including text, if not provided the end of the file is "
                                 "used. By default this line is not included in the display.")
        settings['include-start'] = (True, "When False the texted captured by the 'start' setting "
                                           "is excluded in the displayed text.")
        settings['include-end'] = (False, "When True the texted captured by the 'end' setting is "
                                          "included in the displayed text.")
        return settings

    def createToken(self, info, parent):
        """
        Build the tokens needed for displaying code listing.
        """

        # Locate filename
        filename = common.check_filenames(info['subcommand'])

        # Listing container
        flt = floats.Float(parent)
        self.addCaption(flt)

        # Create code token
        lang = self.settings.get('language')
        lang = lang if lang else common.get_language(filename)
        tokens.Code(flt, style="max-height:{};".format(self.settings['max-height']),
                    code=self.extractContent(filename, self.settings),
                    language=lang)

        # Add bottom modal
        if self.settings['link']:
            rel_filename = os.path.relpath(filename, MooseDocs.ROOT_DIR)
            code = tokens.Code(None, language=lang, code=common.read(filename))
            floats.ModalLink(flt, url=unicode(rel_filename), bottom=True, content=code,
                             string=u'({})'.format(rel_filename),
                             title=tokens.String(None, content=unicode(filename)))

        return parent


    def extractContent(self, filename, settings):
        """
        Extract the desired content from the supplied raw text from a file.

        Inputs:
            filename[unicode]: The file to read (known to exist already).
            settings[dict]: The setting from the createToken method.
        """

        content = common.read(filename)
        if settings['re']:
            content = common.regex(self.settings['re'], content, eval(self.settings['re-flags']))

        elif settings['line']:
            content = self.extractLine(content, settings["line"])

        elif settings['start'] or settings['end']:
            content = self.extractLineRange(content,
                                            settings['start'],
                                            settings['end'],
                                            settings['include-start'],
                                            settings['include-end'])

        return self.prepareContent(content, settings)

    def prepareContent(self, content, settings): #pylint: disable=no-self-use
        """
        Apply the various filters and adjustment to the supplied text.

        Inputs:
            content[unicode]: The extracted content.
            settings[dict]: The setting from the createToken method.
        """
        # Strip leading/trailing newlines
        content = re.sub(r'^(\n*)', '', content)
        content = re.sub(r'(\n*)$', '', content)

        # Strip extra new lines (optional)
        if settings['strip-extra-newlines']:
            content = re.sub(r'(\n{3,})', '\n\n', content)

        # Strip header
        if settings['strip-header']:
            content = re.sub(r'^((?:/{2}|#)\*.*?$)', '', content, flags=re.MULTILINE)

        # Strip leading/trailing white-space
        if settings['strip-leading-whitespace']:
            content = re.sub(r'^(\s+)', '', content, flags=re.MULTILINE)

        # Add indent
        if settings['indent'] > 0:
            replace = r'{}\1'.format(' '*int(settings['indent']))
            content = re.sub(r'^(.*?)$', replace, content)

        # Prefix/suffix
        if settings['header']:
            replace = r'\1{}'.format(settings['header'])
            content = re.sub(r'^(.*?)$', replace, content)

        if settings['footer']:
            replace = r'{}\1'.format(settings['footer'])
            content = re.sub(r'^(.*?)$', replace, content)

        return content

    @staticmethod
    def extractLine(content, desired):
        """
        Function for returning a single line.

        Args:
          conetnt[str]: The string content to examine.
          desired[str]: The text to look for within the source file.
        """

        lines = content.split('\n')

        # Search the lines
        content = None
        for line in lines:
            if desired in line:
                content = line

        return content

    @staticmethod
    def extractLineRange(content, start, end, include_start, include_end):
        """
        Function for extracting content between start/end strings.

        Args:
          conetnt[str]: The string content to examine.
          start[str|None]: The starting line (when None is provided the beginning is used).
          end[str|None]: The ending line (when None is provided the end is used).
          include-start[bool]: If True then the start string is included
          include-end[bool]: If True then the end string is included
        """
        lines = content.split('\n')
        start_idx = 0
        end_idx = len(lines)

        if start:
            for i in range(end_idx):
                if start in lines[i]:
                    start_idx = i if include_start else i+1
                    break
        if end:
            for i in range(start_idx, end_idx):
                if end in lines[i]:
                    end_idx = i + 1 if include_end else i
                    break

        return '\n'.join(lines[start_idx:end_idx])

class InputListingCommand(FileListingCommand):
    """
    Special listing command for MOOSE hit imput files.
    """

    COMMAND = 'listing'
    SUBCOMMAND = 'i'

    @staticmethod
    def defaultSettings():
        settings = FileListingCommand.defaultSettings()
        settings['block'] = (None, 'Space separated list of input file block names to include.')
        return settings

    def extractContent(self, filename, settings):

        if self.settings['block']:
            return self.extractInputBlocks(filename, self.settings['block'])

        return FileListingCommand.extractContent(self, filename, settings)

    @staticmethod
    def extractInputBlocks(filename, blocks):

        hit = mooseutils.hit_load(filename)
        out = []
        for block in blocks.split(' '):
            node = hit.find(block)
            if node is None:
                msg = "Unable to find block '{}' in {}."
                raise exceptions.TokenizeException(msg, block, filename)
            out.append(unicode(node.render()))
        return '\n'.join(out)
