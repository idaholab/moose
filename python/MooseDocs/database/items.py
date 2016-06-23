"""
The following objects are designed to work with the Database class,
see Database.py for usage.
"""
import os
import re
import subprocess
import MooseDocs
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
        yield os.path.basename(self._filename)[0:-3]

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

        for match in re.finditer(self._regex, self.content()):
            yield match.group('key')

class InputFileItem(RegexItem):
    """
    Returns a markdown list item for input file matching of (type = ).
    """
    def __init__(self, filename, **kwargs):
        RegexItem.__init__(self, filename, r'type\s*=\s*(?P<key>\w+)\b', **kwargs)

    def markdown(self):
        return '* [{}]({})'.format(self._rel_path, self._repo)

class ChildClassItem(RegexItem):
    """
    Returns a markdown list item for h file containing a base.
    """
    def __init__(self, filename, **kwargs):
        RegexItem.__init__(self, filename, r'public\s*(?P<key>\w+)\b', **kwargs)

    def markdown(self):
        # Check for C file
        c_rel_path = self._rel_path.replace('/include/', '/src/').replace('.h', '.C')
        c_repo = self._repo.replace('/include/', '/src/').replace('.h', '.C')
        c_filename = self._filename.replace('/include/', '/src/').replace('.h', '.C')

        if os.path.exists(c_filename):
            md = '* [{}]({})<br>[{}]({})'.format(self._rel_path, self._repo, c_rel_path, c_repo)
        else:
            md = '* [{}]({})'.format(self._rel_path, self._repo)

        return md
