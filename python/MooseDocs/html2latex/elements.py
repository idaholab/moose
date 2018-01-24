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

import os
import logging

import bs4

import mooseutils

LOG = logging.getLogger(__name__)

class LatexNavigableString(bs4.element.NavigableString):
    """
    An empty class to allow the Translator to convert the text without escaping content.
    """
    pass

class Element(object):
    """
    Base class for converting html tag to latex.

    The basic conversion by changing the html tags to a "latex" tag and adding meta data to the tag
    attributes. See Translator for meta data use.

    Args:
        name[str]: (Required) The tag name to test against.
        attrs[dict]: (Optional) A dictionary of attributes and values that are required (see test())

    Kwargs (Optional):
        The following keywords are converted to tag meta data for latex conversion. The values
        passed in for each of the keywords should be a str type.

        begin --> data-latex-begin
            The command to place prior to content.
        begin_prefix --> data-latex-begin-prefix
            The text (e.g., '\n') that should be placed prior to the begin command.
        begin_suffix --> data-latex-begin-suffix
            The text (e.g., '\n') that should be placed after to the begin command.
        end --> data-latex-end
            The command to place after the content.
        end_prefix --> data-latex-end-prefix
            The text (e.g., '\n') that should be placed prior to the end command.
        end_suffix --> data-latex-end-suffix
            The text (e.g., '\n') that should be placed after to the end command.
        open -> data-latex-open
            Text placed prior to all begin commands and content.
        close -> data-latex-close
            Text placed after content and all end commands.
        content -> data-latex-content
            Text used to replace the content of the tag including children
    """
    def __init__(self, name=None, attrs=None, strip=False, **kwargs):

        if name is None:
            raise mooseutils.MooseException("The 'name' argument variable must be set.")

        self._name = name
        self._attrs = attrs if attrs is not None else dict()
        self._strip = strip

        self._data = dict()
        keys = ['begin', 'begin_prefix', 'begin_suffix', 'end', 'end_prefix', 'end_suffix', 'open',
                'close', 'content', 'escape']
        for k in keys:
            self._data.setdefault('data-latex-{}'.format(k.replace('_', '-')), kwargs.get(k, None))
        self.__soup = None

    def __call__(self, soup, tag):
        self.__soup = soup
        if self.test(tag):
            self.convert(tag)
            tag.name = 'latex'

    def test(self, tag):
        """
        Return True if the tag is to be converted.

        Inputs:
            tag[bs4.element.Tag]: The current parsed html tag.
        """
        if tag.name == 'latex':
            return False

        if not isinstance(tag, bs4.element.Tag) or tag.name != self._name:
            return False

        for key, value in self._attrs.iteritems():
            if (key not in tag.attrs) or (value not in tag[key]):
                return False

        return True

    @staticmethod
    def strip(tag):
        """
        Strip whitespace from string descendants: lstrip on first and rstrip on last.

        Inputs:
            tag[bs4.element.Tag]: The current parsed html tag.
        """
        strs = list(tag.strings)
        strs[0].replace_with(strs[0].lstrip())
        strs[-1].replace_with(strs[-1].rstrip())

    def convert(self, tag):
        """
        Convert the html tag to a "latex" tag.

        Inputs:
            tag[bs4.element.Tag]: The current parsed html tag.
        """

        tag.name = 'latex'
        for key, value in self._data.iteritems():
            if value is not None:
                tag.attrs.setdefault(key, value)
        if 'data-latex-content' in tag.attrs:
            tag.replace_with(self.new(string=LatexNavigableString(tag.attrs['data-latex-content'])))

        if self._strip:
            self.strip(tag)

    def new(self, name='latex', string=None):
        """
        Create a new bs4.element.Tag object.

        Inputs:
            name[str]: (optional) The name of the tag to create.
            string[str]: (optional) The string content to add to the tag.
        """
        ntag = self.__soup.new_tag(name)
        if string:
            ntag.string = string
        return ntag

    def curly(self, **kwargs):
        """
        Create a latex curly bracket tag.
        """
        ntag = self.new(**kwargs)
        ntag.attrs['data-latex-begin'] = '{'
        ntag.attrs['data-latex-end'] = '}'
        return ntag

    def square(self, **kwargs):
        """
        Create a latex square bracket tag.
        """

        ntag = self.new(**kwargs)
        ntag.attrs['data-latex-begin'] = '['
        ntag.attrs['data-latex-end'] = ']'
        return ntag

class Command(Element):
    """
    Object for creating latex commands (e.g., \\par).
    """
    def __init__(self, command=None, **kwargs):
        super(Command, self).__init__(**kwargs)

        self._command = command
        if self._command is None:
            raise mooseutils.MooseException("The 'command' argument variable must be set.")

        self._data['data-latex-begin'] = '\\{}'.format(self._command)

class ArgumentCommand(Command):
    """
    Object for creating latex commands with an argument (e.g., \\section{foo}).
    """

    def convert(self, tag):
        super(ArgumentCommand, self).convert(tag)
        new = self.curly()
        for child in reversed(tag.contents):
            new.insert(0, child.extract())
        tag.append(new)

class Environment(Command):
    """
    Object for creating latex environment (e.g., \\begin{table} ... \\end{table}).
    """
    def __init__(self, **kwargs):
        kwargs.setdefault('begin_suffix', '\n')
        kwargs.setdefault('end_prefix', '\n')
        super(Environment, self).__init__(**kwargs)
        self._data['data-latex-begin'] = '\\begin{%s}' % self._command
        self._data['data-latex-end'] = '\\end{%s}' % self._command

class Heading(Command):
    """
    Converts html heading tag to latex section.
    """
    def convert(self, tag):
        """
        Creates desired section with label.
        """
        super(Heading, self).convert(tag)
        id_ = tag.get('id', None)
        if id_:
            string = tag.string.wrap(self.curly())

            label = self.new()
            label['data-latex-begin'] = '\\label'
            string.append(label)

            text = self.curly()
            text.string = id_
            label.append(text)

        else:
            tag.string.wrap(self.curly())

class PreCode(Environment):
    """
    Converts <pre><code> blocks to verbatim latex.
    """
    def __init__(self, **kwargs):
        kwargs.setdefault('name', 'pre')
        kwargs.setdefault('command', 'verbatim')
        super(PreCode, self).__init__(**kwargs)

    def test(self, tag):
        """
        Makes sure <code> block is directly within a <pre>
        """
        return super(PreCode, self).test(tag) and (tag.code)

    def convert(self, tag):
        """
        Sets the <code> block to be converted.
        """
        super(PreCode, self).convert(tag)
        tag.code.name = 'latex'

class Table(Environment):
    """
    Creates tablular to html table tag.
    """
    def __init__(self, **kwargs):
        kwargs.setdefault('command', 'tabular')
        super(Table, self).__init__(name='table', **kwargs)

    def convert(self, tag):
        """
        Adds the column settings to the environment.
        """
        super(Table, self).convert(tag)
        tag['data-latex-begin-suffix'] = ''
        cols = self.curly()
        cols.string = 'l'*self.numColumns(tag)
        cols['data-latex-close'] = '\n'
        tag.insert(0, cols)

    @staticmethod
    def numColumns(tag):
        """
        Determines the number of columns.
        """
        return len(tag.tbody.find('tr').find_all('td'))

class TableHeaderFooter(Element):
    """
    thead, tfoot conversion.
    """
    def convert(self, tag):
        """
        Wraps table header and footers with horizontal rule.
        """
        super(TableHeaderFooter, self).convert(tag)
        tag['data-latex-open'] = '\\hline\n'
        tag['data-latex-close'] = '\\hline'

class TableItem(Element):
    """
    Converts td, th tags.
    """
    def convert(self, tag):
        """
        Adds closing '&' or '\\\\' to a table item.
        """
        super(TableItem, self).convert(tag)
        if tag.find_next_sibling(self._name):
            tag['data-latex-close'] = ' & '
        else:
            tag['data-latex-close'] = ' \\\\'

class ListItem(Command):
    """
    Convert li tag.
    """
    def __init__(self, **kwargs):
        super(ListItem, self).__init__(name='li', command='item', **kwargs)

    def convert(self, tag):
        """
        Adds a new-line to close the tag for the last item.
        """
        super(ListItem, self).convert(tag)
        tag['data-latex-begin-suffix'] = ' '
        if tag.find_next_sibling(self._name):
            tag['data-latex-close'] = '\n'

class Image(ArgumentCommand):
    """
    Converts <img> tag.
    """
    def __init__(self, **kwargs):
        kwargs.setdefault('end_suffix', '\n')
        super(Image, self).__init__(name='img',
                                    command='includegraphics',
                                    attrs={'src':''},
                                    **kwargs)

    def convert(self, tag):
        """
        Places the filename in the includegraphics command and errors if the file is not found.
        """
        tag.string = tag['src']
        super(Image, self).convert(tag)

        if not os.path.exists(tag.string):
            LOG.error('Image file does not exist: %s', tag.string)

class Figure(Environment):
    """
    Convers <figure> tag.
    """
    def __init__(self, **kwargs):
        kwargs.setdefault('name', 'figure')
        kwargs.setdefault('command', 'figure')
        super(Figure, self).__init__(**kwargs)
    def convert(self, tag):
        """
        Adds label to the figure.
        """
        super(Figure, self).convert(tag)
        if 'id' in tag.attrs:
            label = self.curly()
            label.attrs['data-latex-begin-prefix'] = '\\label'
            label.attrs['data-latex-end-suffix'] = '\n'
            label.string = tag.attrs['id']
            tag.insert(0, label)
        else:
            tag['data-latex-begin'] = '\\begin{%s*}' % self._command
            tag['data-latex-end'] = '\\end{%s*}' % self._command


class LinkElement(ArgumentCommand):
    """
    Convert <a> to hyperlink.
    """
    def __init__(self, **kwargs):
        super(LinkElement, self).__init__(name='a', attrs={'href':''}, command='href', **kwargs)

    def convert(self, tag):
        """
        Extracts the tag href attribute and adds to \\href{} command.
        """
        super(LinkElement, self).convert(tag)
        url = self.curly()
        url.string = tag.get('href', '#')
        tag.insert(0, url)
