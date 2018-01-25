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
import collections
import logging

import jinja2
from markdown.util import etree
from markdown.inlinepatterns import Pattern
from markdown.preprocessors import Preprocessor
from markdown.blockprocessors import BlockProcessor

import mooseutils

import MooseDocs
from MooseMarkdownExtension import MooseMarkdownExtension
from MooseMarkdownCommon import MooseMarkdownCommon

LOG = logging.getLogger(__name__)

class SQAExtension(MooseMarkdownExtension):
    """
    Extension for create software quality documents from a template markdown file.
    """
    @staticmethod
    def defaultConfig():
        """
        Default configuration options for SQAExtension
        """
        config = MooseMarkdownExtension.defaultConfig()
        config['PROJECT'] = ['UNKNOWN PROJECT', "Project name to use throughout the template."]
        config['repo'] = ['', "The remote repository to create hyperlinks."]
        config['branch'] = ['master', "The branch name to consider in repository links."]
        return config

    def extendMarkdown(self, md, md_globals):
        """
        Adds components to SQAExtension.
        """
        md.registerExtension(self)
        config = self.getConfigs()

        repo = os.path.join(config['repo'], 'blob', config['branch'])
        database = SQAInputTagDatabase(repo)

        md.preprocessors.add('moose_sqa',
                             SQAPreprocessor(markdown_instance=md, **config), '_begin')

        md.parser.blockprocessors.add('moose_sqa_input_tags',
                                      SQAInputTags(markdown_instance=md, **config),
                                      '_begin')

        md.inlinePatterns.add('moose_sqa_matrix',
                              SQAInputTagMatrix(markdown_instance=md, database=database, **config),
                              '_begin')

        md.inlinePatterns.add('moose-sqa-page-status',
                              SQAPageStatus(markdown_instance=md, **config),
                              '_begin')

        md.inlinePatterns.add('moose-sqa-link',
                              SQALink(markdown_instance=md, **config),
                              '_begin')


def makeExtension(*args, **kwargs): #pylint: disable=invalid-name
    """
    Create SQAExtension
    """
    return SQAExtension(*args, **kwargs)

class SQAPreprocessor(MooseMarkdownCommon, Preprocessor):
    """
    Preprocessor to read the template and create the complete markdown content.
    """

    SQA_LOAD_RE = r'(?<!`)!SQA-load\s+(?P<filename>.*\.md)(?:$|\s+)(?P<settings>.*)'
    SQA_TEMPLATE_RE = r'!SQA-template\s+(?P<name>\w+)(?P<settings>.*?)' \
                      r'\n(?P<markdown>.*?)!END-template'

    @staticmethod
    def defaultSettings():
        """Default settings for SQAPreprocessor"""
        settings = dict()
        settings['default'] = [False, "Use the template text as the default."]
        return settings

    def __init__(self, markdown_instance=None, **kwargs):
        MooseMarkdownCommon.__init__(self)
        Preprocessor.__init__(self, markdown_instance)
        self.__database = None
        self._template_args = kwargs

    def run(self, lines):
        """
        Load the SQA template file and replace content.
        """

        # Do nothing if the document doesn't contain the template name at the top
        match = re.search(self.SQA_LOAD_RE, lines[0])
        if not match:
            return lines

        # Populate the data base
        self.__database = SQADatabase('\n'.join(lines))

        # Define the template locations
        paths = [MooseDocs.ROOT_DIR,
                 os.path.join(MooseDocs.ROOT_DIR, 'doc', 'templates', 'sqa'),
                 os.path.join(MooseDocs.ROOT_DIR, 'docs', 'templates', 'sqa'),
                 MooseDocs.MOOSE_DIR,
                 os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'templates', 'sqa')]

        # Read the SQA template filename
        filename = match.group('filename')
        for root in paths:
            fullname = os.path.join(root, filename)
            if os.path.exists(fullname):
                filename = fullname
                break

        if not os.path.exists(filename):
            raise IOError("The file '{}' does not exist.".format(filename))

        with open(filename, 'r') as fid:
            content = fid.read()

        # Replace the SQA template sections with content
        content = re.sub(self.SQA_TEMPLATE_RE, self.__sub, content, flags=re.DOTALL|re.MULTILINE)
        template = jinja2.Template(content)
        content = template.render(**self._template_args)
        return content.split('\n')

    def __sub(self, match):
        """
        Substitution method for regex replacement.
        """

        name = match.group('name')
        markdown = match.group('markdown')
        settings = self.getSettings(match.group('settings'))

        # Use the content in database
        if name in self.__database:
            div = etree.Element('div')
            div.set('markdown', '1')
            item = self.__database[name]
            div.text = item.markdown

        # Use the default
        elif settings['default']:
            div = etree.Element('div')
            div.set('markdown', '1')
            div.text = markdown

        # Produce error
        else:

            help_div = etree.Element('div')
            heading = etree.SubElement(help_div, 'h3')
            heading.text = "Adding Markdown for '{}' Item.".format(name)

            p = etree.SubElement(help_div, 'p')
            p.text = "To add content for the '{}' item, simply add a block similar to what is ' \
                     'shown below in the markdown file '{}'.".format(name,
                                                                     self.markdown.current.filename)

            pre = etree.SubElement(help_div, 'pre')
            code = etree.SubElement(pre, 'code')
            code.set('class', 'language-text')
            code.text = '!SQA-template-item {}\nThe content placed here should be valid markdown ' \
                        'that will replace the template description.\n!END-template-item' \
                        .format(name)

            title = 'Missing Template Item: {}'.format(name)
            div = self.createErrorElement(title=title, message=markdown, markdown=True,
                                          help_button=help_div)

        return etree.tostring(div)

    @staticmethod
    def createModalElement(parent, button=None, id_=None, heading=None, color='blue'):
        """
        Creates the modal help box.
        """

        a = etree.Element('a')
        a.set('class', 'waves-effect waves-light btn-floating {}'.format(color))
        a.set('data-target', id_)
        i = etree.SubElement(a, 'i')
        i.set('class', 'material-icons')
        i.text = button

        modal = etree.SubElement(parent, 'div')
        modal.set('id', id_)
        modal.set('class', 'modal')

        content = etree.SubElement(modal, 'div')
        content.set('class', 'modal-content')

        if heading:
            h = etree.SubElement(content, 'h3')
            h.text = heading

        div = etree.SubElement(content, 'div')

        return div, a

class SQADatabase(object):
    """
    Helper object for creating a database of items.
    """

    SQA_ITEM_RE = r'!SQA-template-item\s+(?P<name>\w+)(?P<settings>.*?)' \
                  r'\n(?P<markdown>.*?)!END-template-item'
    ITEMINFO = collections.namedtuple('ItemInfo', 'name settings markdown')

    def __init__(self, content):
        self.__items = dict()
        re.sub(self.SQA_ITEM_RE, self.__sub, content, flags=re.DOTALL|re.MULTILINE)

    def __sub(self, match):
        """
        Regex sub method (see __init__)
        """

        name = match.group('name')
        markdown = match.group('markdown')

        settings = dict()
        for entry in re.findall(MooseMarkdownCommon.SETTINGS_RE, match.group('settings')):
            settings[entry[0].strip()] = entry[1].strip()

        if name in self.__items:
            msg = "The supplied SQA item name ({}) is already defined.".format(name)
            raise mooseutils.MooseException(msg)

        self.__items[name] = self.ITEMINFO(name=name, markdown=markdown, settings=settings)

    def __getitem__(self, name):
        """
        [] operator access to content
        """
        return self.__items[name]

    def __contains__(self, name):
        """
        "in" operator testing for content
        """
        return name in self.__items

class SQAInputTags(MooseMarkdownCommon, BlockProcessor):
    """
    Adds command for defining groups of requirements, validation, etc. lists that are linked to
    input files.
    """
    RE = re.compile(r'(?<!`)!SQA-(?P<key>\w+)-list' \
                    r'(?:$|(?P<settings>.*?))\n' \
                    r'(?P<items>.*?)(?:\Z|\n{2,})',
                    flags=re.DOTALL)

    @staticmethod
    def defaultSettings():
        """Settings for SQARequirement"""
        settings = MooseMarkdownCommon.defaultSettings()
        settings['title'] = ('', "Title to assign to the list of items.")
        settings['require-markdown'] = (False, "")
        settings['status'] = (False, "When enabled with 'require-markdown' the status of the " \
                                     "page is added to the list.")
        settings['markdown-folder'] = ('', "When supplied the value provided should be a " \
                                           "directory relative to the repository root directory " \
                                           "where the required files are located " \
                                           "(require-markdown must be enabled).")
        return settings

    def __init__(self, markdown_instance=None, **kwargs):
        MooseMarkdownCommon.__init__(self, **kwargs)
        BlockProcessor.__init__(self, markdown_instance.parser)
        self.markdown = markdown_instance

    def test(self, parent, block):
        """
        Check that block contains the defined RE.
        """
        return self.RE.search(block)

    def run(self, parent, blocks):
        """
        Create the collapsible region with the listed requirements.
        """
        block = blocks.pop(0)
        match = self.RE.search(block)
        settings = self.getSettings(match.group('settings'))
        key = match.group('key')

        # Set the default directory
        require_md = settings['require-markdown']
        status = settings['status']
        if require_md:
            folder = settings['markdown-folder']
            if not folder:
                folder = os.path.join(os.path.dirname(self.markdown.current.filename), key)
            else:
                folder = os.path.join(MooseDocs.ROOT_DIR, folder)

        ul = MooseDocs.common.MooseCollapsible()
        if settings['title']:
            ul.addHeader(settings['title'])

        for item in match.group('items').split('\n'):
            tag_id, text = re.split(r'\s+', item.strip(), maxsplit=1)
            desc = text
            if require_md:
                slug, _ = MooseDocs.common.slugify(tag_id, ('.', '-'))
                slug += '.md'
                full_name = os.path.join(folder, slug)
                if not os.path.exists(full_name):
                    LOG.error("The required markdown file (%s) does not exist.", full_name)
                tag_id = '<a href="{}">{}</a>'.format(full_name, tag_id)
                if status:
                    _, found = self.getFilename(slug)
                    if found:
                        status_el = SQAPageStatus.createStatusElement(found, self.markdown.current)
                        desc += etree.tostring(status_el)

            ul.addItem(tag_id, desc, text, id_='{}-{}'.format(key, tag_id))

        parent.append(ul.element())

class SQAInputTagMatrix(MooseMarkdownCommon, Pattern):
    """
    Adds input tag matrix creation (e.g., Requirements Traceability Matrix)
    """
    RE = r'(?<!`)!SQA-(?P<key>\w+)-matrix\s+(?P<filename>.*\.md)(?:\n|\s*(?P<settings>.*))'

    @staticmethod
    def defaultSettings():
        settings = MooseMarkdownCommon.defaultSettings()
        return settings

    def __init__(self, markdown_instance=None, database=None, **kwargs):
        MooseMarkdownCommon.__init__(self, **kwargs)
        Pattern.__init__(self, self.RE, markdown_instance)
        self._database = database

    def handleMatch(self, match):
        """
        Build the matrix.
        """
        # Determine the items to include in the matrix
        key = match.group('key')

        # Report error if the key is not located
        if key not in self._database:
            return self.createErrorElement("The desired key ({0}) does not exist in any input " \
                                           "file using the expected '@{0} <tag>' syntax." \
                                           .format(key))

        # Load the desired items
        filename = match.group('filename')
        full_file, _ = self.markdown.getFilename(filename, check_local=True)
        if os.path.exists(full_file):
            items = self.getItems(full_file, key)
        else:
            return self.createErrorElement("Unable to locate markdown file {}.".format(filename))

        # Create the table
        ul = MooseDocs.common.MooseCollapsible()
        for title in sorted(items.keys()):
            ul.addHeader(title)
            for tag_id, text in items[title]:
                tag, desc, body = self.buildText(key, filename, tag_id, text)
                ul.addItem(tag, desc, body)

        # Return the element
        return ul.element()

    @staticmethod
    def getItems(filename, key):
        """
        Extract the matrix items from a markdown file.
        """
        out = collections.defaultdict(list)
        with open(filename) as fid:
            for match in SQAInputTags.RE.finditer(fid.read()):
                if key == match.group('key'):
                    options = dict(title='')
                    settings = MooseMarkdownCommon.getSettingsHelper(options,
                                                                     match.group('settings'),
                                                                     legacy_style=False)
                    title = settings['title']
                    for item in match.group('items').split('\n'):
                        tag_id, text = re.split(r'\s+', item.strip(), maxsplit=1)
                        out[title].append((tag_id, text))
        return out

    def buildText(self, key, filename, tag_id, text):
        """
        Create the content for the collapsible body
        """
        if tag_id not in self._database[key]:
            tag = '<span class="moose-sqa-error">{}</span>'.format(tag_id)
            desc = '<span class="moose-sqa-error">{}</span>'.format(text)
            body = desc
        else:
            desc = '<div class="moose-sqa-list-description">{}</div>'.format(text)
            body = desc
            tag = '<a href="{}">{}</a>'.format(filename, tag_id)
            for item in self._database[key][tag_id]:
                body += '<div class="moose-sqa-list-link"><a href="{}">{}</a></div>'.format(*item)
        return tag, desc, body

class SQAInputTagDatabase(object):
    """
    Storage container for SQA tagged groups (e.g., requirements).

    This storage is used for recall by the traceability matrix.
    """
    RE = re.compile(r'@(?P<key>\w+)\s+(?P<tag>.*?)(?:$|\s+)', flags=re.IGNORECASE)

    def __init__(self, repo):
        self.__repo = repo
        self.__database = collections.defaultdict(lambda: collections.defaultdict(list))
        for base, _, files in os.walk(MooseDocs.ROOT_DIR):
            for filename in files:
                full_name = os.path.join(base, filename)
                if filename.endswith('.i'):
                    self.search(full_name)

    def __getitem__(self, value):
        """
        Operator[] access to the top-level of the storage dict.
        """
        return self.__database[value]

    def __contains__(self, key):
        """
        Keyword "in" access to the top-level of the storage dict.
        """
        return key in self.__database

    def search(self, filename):
        """
        Locates @<keyword> tags within input file.
        """
        with open(filename) as fid:
            for match in self.RE.finditer(fid.read()):
                key = match.group('key').lower()
                tag = match.group('tag')
                local = filename.replace(MooseDocs.ROOT_DIR, '').lstrip('/')
                remote = '{}/{}'.format(self.__repo.rstrip('/'), local)
                self.__database[key][tag].append((remote, local))

class SQAPageStatus(MooseMarkdownCommon, Pattern):
    """
    Creates tag that displays the pages status, which is updated from a script in init.js
    """
    RE = r'(?<!`)!SQA-status\s*(?P<filename>.*?)!'

    @staticmethod
    def defaultSettings():
        settings = MooseMarkdownCommon.defaultSettings()
        return settings

    def __init__(self, markdown_instance=None, **kwargs):
        MooseMarkdownCommon.__init__(self, **kwargs)
        Pattern.__init__(self, self.RE, markdown_instance)

    def handleMatch(self, match):
        """
        Return a div that will be replaced.
        """
        _, found = self.markdown.getFilename(match.group('filename').strip())
        if not found:
            return self.createErrorElement("Unable to locate filename {} in !status command." \
                                           .format(match.group('filename')))
        return self.createStatusElement(found, self.markdown.current)

    @staticmethod
    def createStatusElement(found, current):
        """
        Helper for creating status span tag.
        """
        el = etree.Element('span')
        el.set('class', 'moose-page-status')
        local = os.path.relpath(found.destination, os.path.dirname(current.destination))
        el.set('data-filename', local)
        return el

class SQALink(MooseMarkdownCommon, Pattern):
    """
    Creates markdown link syntax that accept key, value pairs.
    """
    RE = r'(?<!`)\[(?P<text>.*?)\]\((?P<filename>.*?)(?:\s+(?P<settings>.*?))?\)'

    @staticmethod
    def defaultSettings():
        settings = MooseMarkdownCommon.defaultSettings()
        settings['status'] = (False, "When True status badge(s) are created that display the "
                                     "status of the linked page.")
        return settings

    def __init__(self, markdown_instance=None, **kwargs):
        MooseMarkdownCommon.__init__(self, **kwargs)
        Pattern.__init__(self, self.RE, markdown_instance)

    def handleMatch(self, match):
        """
        Return a div that will be replaced.
        """
        # Do nothing if settings not given
        if not match.group('settings'):
            return None

        # Extract data
        settings = self.getSettings(match.group('settings'))
        filename = match.group('filename')
        text = match.group('text')

        # Create a element
        el = etree.Element('a')
        el.set('href', match.group('filename'))
        if settings['status']:
            span = etree.SubElement(el, 'span')
            span.text = text
            _, found = self.markdown.getFilename(filename)
            if found:
                el.append(SQAPageStatus.createStatusElement(found, self.markdown.current))
        else:
            el.text = text
        return el
