#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################
#pylint: enable=missing-docstring
import re
import os
import collections

import jinja2
from markdown.util import etree
from markdown.preprocessors import Preprocessor

import mooseutils

import MooseDocs
from MooseMarkdownExtension import MooseMarkdownExtension
from MooseMarkdownCommon import MooseMarkdownCommon

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
        return config

    def extendMarkdown(self, md, md_globals):
        """
        Adds components to SQAExtension.
        """
        md.registerExtension(self)
        config = self.getConfigs()
        md.preprocessors.add('moose_sqa',
                             SQAPreprocessor(markdown_instance=md, **config), '_begin')

def makeExtension(*args, **kwargs): #pylint: disable=invalid-name
    """
    Create SQAExtension
    """
    return SQAExtension(*args, **kwargs)

class SQAPreprocessor(MooseMarkdownCommon, Preprocessor):
    """
    Preprocessor to read the template and create the complete markdown content.
    """

    SQA_LOAD_RE = r'(?<!`)!SQA\s+(?P<filename>.*\.md)(?:$|\s+)(?P<settings>.*)'
    SQA_TEMPLATE_RE = r'!SQA\s+template\s+(?P<name>\w+)(?P<settings>.*?)\n(?P<markdown>.*?)!END'

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
            code.text = '!SQA item {}\nThe content placed here should be valid markdown that ' \
                        'will replace the template description.\n!END'.format(name)

            title = 'Missing Template Item: {}'.format(name)
            div = MooseMarkdownCommon.createErrorElement(title=title,
                                                         message=markdown,
                                                         markdown=True,
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

    SQA_ITEM_RE = r'!SQA\s+item\s+(?P<name>\w+)(?P<settings>.*?)\n(?P<markdown>.*?)!END'
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
