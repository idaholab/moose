"""
The following objects are designed to work with the Database class,
see Database.py for usage.
"""
import os
import re
import subprocess
import MooseDocs
from markdown.util import etree
import logging
log = logging.getLogger(__name__)

class DatabaseItem(object):
    """
    Base class for database items.

    Args:
        filename[str]: The complete filename (supplied by Database object).
    """

    output = os.path.dirname(subprocess.check_output(['git', 'rev-parse', '--git-dir'], stderr=subprocess.STDOUT))

    def __init__(self, filename, **kwargs):
        self._filename = os.path.abspath(filename)
        self._rel_path = os.path.relpath(filename, self.output)
        self._config = kwargs

    def keys(self):
        pass

    def markdown(self):
        pass

    def html(self):
        pass

    def filename(self):
        return self._filename

    def content(self):
        fid = open(self._filename, 'r')
        content = fid.read()
        fid.close()
        return content


class MarkdownIncludeItem(DatabaseItem):
    """
    An item that returns a markdown include string for use with the markdown_include extension.
    """

    def keys(self):
        yield self._filename

    def markdown(self):
        return '{{!{}!}}'.format(self._filename)


class RegexItem(DatabaseItem):
    """
    An item that creates keys base on regex match.
    """

    def __init__(self, filename, regex, **kwargs):
        DatabaseItem.__init__(self, filename, **kwargs)

        self._regex = re.compile(regex)
        self._repo = os.path.join(kwargs.get('repo'), self._rel_path)

    def keys(self):
        """
        Return the keys for which this item will be stored in the database.
        """
        keys = []
        for match in re.finditer(self._regex, self.content()):
            k = match.group('key')
            if k not in keys:
                keys.append(k)
        return keys


class InputFileItem(RegexItem):
    """
    Returns a list item for input file matching of (type = ).
    """
    def __init__(self, filename, **kwargs):
        RegexItem.__init__(self, filename, r'type\s*=\s*(?P<key>\w+)\b', **kwargs)

    def markdown(self):
        return '* [{}]({})'.format(self._rel_path, self._repo)

    def html(self):
        el = etree.Element('li')
        a = etree.SubElement(el, 'a')
        a.set('href', self._repo)
        a.text = self._rel_path
        return el

class ChildClassItem(RegexItem):
    """
    Returns a list item for h file containing a base.
    """
    def __init__(self, filename, **kwargs):
        super(ChildClassItem, self).__init__(filename, r'public\s*(?P<key>\w+)\b', **kwargs)

    def html(self, element='li'):
        c_filename = self._filename.replace('/include/', '/src/').replace('.h', '.C')
        el = etree.Element(element)
        a = etree.SubElement(el, 'a')
        a.set('href', self._repo)
        a.text = self._rel_path
        if os.path.exists(c_filename):
            etree.SubElement(el, 'br')
            c_rel_path = self._rel_path.replace('/include/', '/src/').replace('.h', '.C')
            c_repo = self._repo.replace('/include/', '/src/').replace('.h', '.C')
            a = etree.SubElement(el, 'a')
            a.set('href', c_repo)
            a.text = c_rel_path
        return el
