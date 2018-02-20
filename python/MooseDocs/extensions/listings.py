#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
#pylint: enable=missing-docstring

import re
import os
import cgi
import logging

import markdown
from markdown.inlinepatterns import Pattern
from markdown.util import etree
from markdown.extensions.fenced_code import FencedBlockPreprocessor

import MooseDocs
from MooseMarkdownExtension import MooseMarkdownExtension
from MooseMarkdownCommon import MooseMarkdownCommon

# pylint runs during the precheck before the hit python bindings have been build
# so we need to disable import error checking here.
# pylint: disable=import-error
import hit
# pylint: enable=import-error

LOG = logging.getLogger(__name__)

class ListingExtension(MooseMarkdownExtension):
    """
    Extension for adding including code and other text files.
    """
    @staticmethod
    def defaultConfig():
        config = MooseMarkdownExtension.defaultConfig()
        config['repo'] = ['', "The remote repository to create hyperlinks."]
        config['branch'] = ['master', "The branch name to consider in repository links."]
        config['make_dir'] = [MooseDocs.ROOT_DIR,
                              "The location of the MakeFile for determining the include " \
                              "paths when using clang parser."]
        return config

    def extendMarkdown(self, md, md_globals):
        """
        Adds include support for MOOSE flavored markdown.
        """
        md.registerExtension(self)
        config = self.getConfigs()

        # Replace the standard fenced code to a version that handles !listing line
        if 'fenced_code_block' in md.preprocessors:
            FencedBlockPreprocessor.LANG_TAG = ' class="language-%s"'
            md.preprocessors.add('moose_fenced_code_block',
                                 ListingFencedBlockPreprocessor(md),
                                 "<fenced_code_block")

        md.inlinePatterns.add('moose-listing',
                              ListingPattern(markdown_instance=md, **config),
                              '_begin')

        md.inlinePatterns.add('moose-input-listing',
                              ListingInputPattern(markdown_instance=md, **config),
                              '_begin')


def makeExtension(*args, **kwargs): #pylint: disable=invalid-name
    """Create ListingExtension"""
    return ListingExtension(*args, **kwargs)


class ListingPattern(MooseMarkdownCommon, Pattern):
    """
    The basic object for creating code listings from files.

    Args:
      language[str]: The code language (e.g., 'python' or 'c++')
    """
    RE = r'(?<!`)!listing\s+(?P<filename>.*\.\w+)(?:$|\s+)(?P<settings>.*)'

    @staticmethod
    def defaultSettings():
        settings = MooseMarkdownCommon.defaultSettings()
        settings['strip-header'] = (True, "When True the MOOSE header is removed for display.")
        settings['caption'] = (None, "The text caption, if an empty string is provided a link to "
                                     "the filename is created, if None is provided no caption is "
                                     "applied, otherwise the text given is used.")
        settings['language'] = (None, "The language to utilize for providing syntax highlighting.")
        settings['link'] = (True, "Include a link to the filename in the caption.")
        settings['strip-extra-newlines'] = (True, "Removes extraneous new lines from the text.")
        settings['prefix'] = ('', "Text to include prior to the included text.")
        settings['suffix'] = ('', "Text to include after to the included text.")
        settings['indent'] = (0, "The level of indenting to apply to the included text.")
        settings['strip-leading-whitespace'] = (False, "When True leading white-space is removed "
                                                       "from the included text.")
        settings['counter'] = ('listing', "The counter group to associate wit this command.")
        settings['line'] = (None, "A portion of text that unique identifies a single line to "
                                  "include.")
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
        settings['pre-style'] = ("overflow-y:scroll;max-height:350px",
                                 "Style attributes to apply to the code area.")
        return settings

    def __init__(self, markdown_instance=None, **kwargs):
        MooseMarkdownCommon.__init__(self, **kwargs)
        Pattern.__init__(self, self.RE, markdown_instance)

        # The root/repo settings
        self._repo = kwargs.pop('repo')
        self._branch = kwargs.pop('branch')

    def handleMatch(self, match):
        """
        Process the text file provided.
        """
        # Update the settings from g match
        settings = self.getSettings(match.group(3))

        # Read the file
        rel_filename = match.group('filename').lstrip('/')
        filename = os.path.join(self.markdown.current.root_directory, rel_filename)
        if not os.path.exists(filename):
            msg = "Unable to locate file {}".format(rel_filename)
            msg += " in {}".format(self.markdown.current.filename)
            return self.createErrorElement(msg)

        # Figure out file extensions
        if settings['language'] is None:
            _, ext = os.path.splitext(rel_filename)
            if ext in ['.C', '.h', '.cpp', '.hpp']:
                settings['language'] = 'cpp'
            elif ext == '.py':
                settings['language'] = 'python'
            else:
                settings['language'] = 'text'

        # Extract the content from the file
        content = self.extractContent(filename, settings)
        if content is None:
            return self.createErrorElement("Failed to extract content from {}.".format(filename))

        # Apply additional settings to content
        content = self.prepareContent(content, settings)

        # Return the Element object
        el = self.createElement(content, rel_filename, settings)
        return el

    @staticmethod
    def prepareContent(content, settings):
        """
        Prepare the convent for conversion to Element object.

        Args:
          content[str]: The content to prepare (i.e., the file contents).
        """

        # Strip leading/trailing newlines
        content = re.sub(r'^(\n*)', '', content)
        content = re.sub(r'(\n*)$', '', content)

        # Strip extra new lines (optional)
        if settings['strip-extra-newlines']:
            content = re.sub(r'(\n{3,})', '\n\n', content)

        # Strip header
        if settings['strip-header']:
            strt = content.find('/********')
            stop = content.rfind('*******/\n')
            content = content.replace(content[strt:stop+9], '')

        # Strip leading/trailing white-space
        if settings['strip-leading-whitespace']:
            content = re.sub(r'^(\s+)', '', content, flags=re.MULTILINE)

        # Add indent
        if settings['indent'] > 0:
            lines = content.split('\n')
            c = []
            for line in lines:
                c.append('{}{}'.format(' '*int(settings['indent']), line))
            content = '\n'.join(c)

        # Prefix/suffix
        if settings['prefix']:
            content = '{}\n{}'.format(settings['prefix'], content)
        if settings['suffix']:
            content = '{}\n{}'.format(content, settings['suffix'])

        return content

    def createElement(self, content, rel_filename, settings):
        """
        Create the code element from the supplied source code content.

        Args:
          label[str]: The label supplied in the regex, [label](...)
          content[str]: The code content to insert into the markdown.
          filename[str]: The complete filename (for error checking)
          rel_filename[str]: The relative filename; used for creating github link.
          settings[dict]: The current settings.

        NOTE: The code related settings and clean up are applied in this method.
        """

        # Build outer div container
        el = self.createFloatElement(settings)

        # Build the code
        pre = etree.SubElement(el, 'pre')
        code = etree.SubElement(pre, 'code')
        if settings['language']:
            code.set('class', 'language-{}'.format(settings['language']))
        content = cgi.escape(content, quote=True)
        code.text = self.markdown.htmlStash.store(content.strip('\n'))

        # Filename link
        if settings['link']:
            link = etree.Element('div')
            a = etree.SubElement(link, 'a')
            a.set('href', os.path.join(self._repo, 'blob', self._branch, rel_filename))
            a.text = '({})'.format(os.path.basename(rel_filename))
            a.set('class', 'moose-listing-link tooltipped')
            a.set('data-tooltip', rel_filename)
            el.append(link)

        # Code style
        if settings['pre-style']:
            pre.set('style', settings['pre-style'])

        return el

    def extractContent(self, filename, settings):
        """
        Extract the content to display.

        Args:
            filename[str]: The absolute filename to read.
            settings[dict]: The settings for the match.
        """
        content = None
        if settings['line']:
            content = self.extractLine(filename, settings["line"])

        elif settings['start'] or settings['end']:
            content = self.extractLineRange(filename,
                                            settings['start'],
                                            settings['end'],
                                            settings['include-start'],
                                            settings['include-end'])

        else:
            with open(filename) as fid:
                content = fid.read()

        return content

    @staticmethod
    def extractLine(filename, desired):
        """
        Function for returning a single line.

        Args:
          desired[str]: The text to look for within the source file.
        """

        # Read the lines
        with open(filename) as fid:
            lines = fid.readlines()

        # Search the lines
        content = None
        for line in lines:
            if desired in line:
                content = line

        return content

    @staticmethod
    def extractLineRange(filename, start, end, include_start, include_end):
        """
        Function for extracting content between start/end strings.

        Args:
          filename[str]: The name of the file to examine.
          start[str|None]: The starting line (when None is provided the beginning is used).
          end[str|None]: The ending line (when None is provided the end is used).
          include-start[bool]: If True then the start string is included
          include-end[bool]: If True then the end string is included
        """

        # Read the lines
        with open(filename) as fid:
            lines = fid.readlines()

        # The default start/end positions
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

        return ''.join(lines[start_idx:end_idx])

class ListingInputPattern(ListingPattern):
    """
    Markdown extension for extracting blocks from input files.
    """
    RE = r'(?<!`)!listing\s+(?P<filename>.*\.i)(?:$|\s+)(?P<settings>.*)'

    @staticmethod
    def defaultSettings():
        settings = ListingPattern.defaultSettings()
        settings['block'] = (None, "The input file syntax block to include.")
        settings['main_comment'] = (False, "When True the main comment block (all text prior to "
                                           "first block) is included for display.")
        return settings

    def __init__(self, **kwargs):
        super(ListingInputPattern, self).__init__(**kwargs)

    def extractContent(self, filename, settings):
        """
        Extract input file content with GetPot parser if 'block' is available. (override)
        """
        if settings['main_comment']:
            with open(filename) as fid:
                content = fid.read()
            match = re.search(r'(?P<comment>.*?)\[.*?\]', content, flags=re.MULTILINE|re.DOTALL)
            return match.group('comment')

        if not settings['block']:
            return super(ListingInputPattern, self).extractContent(filename, settings)

        with open(filename) as f:
            content = f.read()

        root = hit.parse(filename, content)
        node = root.find(settings['block'])
        if node is not None:
            return node.render()

        return super(ListingInputPattern, self).extractContent(filename, settings)

class ListingFencedBlockPreprocessor(FencedBlockPreprocessor, MooseMarkdownCommon):
    """
    Adds the ability to proceed a fenced code block with !listing command.
    """
    RE = r'(?<!`)!listing\s*(?P<settings>.*)'
    LANG_TAG = ' class="language-%s"'

    @staticmethod
    def defaultSettings():
        settings = MooseMarkdownCommon.defaultSettings()
        settings['caption'] = (None, "The caption text to place after the heading and number.")
        settings['counter'] = ('listing', "The name of global counter to utilized for numbering.")
        return settings

    def __init__(self, markdown_instance=None, **config):
        MooseMarkdownCommon.__init__(self, **config)
        FencedBlockPreprocessor.__init__(self, markdown_instance)
        self._remove = []

    def sub(self, text, match):
        """
        Substitution function that captures !listings command above a fenced code block.
        """

        idx = text.rfind('\n', 0, match.start(0)-1)
        if idx:
            listing = re.search(self.RE, text[idx+1:match.start(0)-1])
            if listing:

                # Stash the !listing line to be removed
                self._remove.append(text[idx+1:match.start(0)-1])

                # The html stash that will contain the fenced code block
                placeholder = markdown.util.HTML_PLACEHOLDER % self.markdown.htmlStash.html_counter

                # Build the containing <div>
                settings = self.getSettings(listing.group('settings'))
                div = self.createFloatElement(settings)

                # Parse the fenced code block
                lines = match.group(0).split('\n')
                lines = super(ListingFencedBlockPreprocessor, self).run(lines)

                # String of html tags to wrap fenced content with
                start = self.markdown.htmlStash.store(etree.tostring(div)[:-6], safe=True)
                end = self.markdown.htmlStash.store('</div>', safe=True)

                # Add the wrapped html around the fenced block
                idx = lines.index(placeholder)
                lines.insert(idx+1, end)
                lines.insert(idx, start)
                return '\n'.join(lines)

        return match.group(0)

    def run(self, lines):
        """
        Preprocess the lines by wrapping the listing <div> around the fenced code.
        """

        self._remove = []

        text = '\n'.join(lines)
        text = self.FENCED_BLOCK_RE.sub(lambda m: self.sub(text, m), text)

        for s in self._remove:
            text = text.replace(s, '')

        # Restore the code wrapping
        self.CODE_WRAP = '<pre><code%s>%s</code></pre>'#pylint: disable=invalid-name
        return text.split('\n')
