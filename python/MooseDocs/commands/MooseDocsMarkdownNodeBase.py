import os
import logging
log = logging.getLogger(__name__)

import MooseDocs
from MooseDocsNode import MooseDocsNode

class MooseDocsMarkdownNodeBase(MooseDocsNode):
    """
    Node for converting markdown to html.
    """
    def __init__(self, md_file=None, parser=None, **kwargs):
        super(MooseDocsMarkdownNodeBase, self).__init__(**kwargs)

        if (not md_file) or (not os.path.exists(md_file)):
            raise Exception('The supplied markdown file must exists: {}'.format(md_file))

        # Extract the MooseLinkDatabase for creating source and doxygen links
        ext = MooseDocs.get_moose_markdown_extension(parser)
        self._syntax = ext.syntax if ext else dict()

        self._parser = parser
        self._md_file = md_file

    def source(self):
        """
        Return the source markdown file.
        """
        return self._md_file

    def build(self, lock=None):
        """
        Converts the markdown to html.
        """
        content = self._parser.convert(self)
        self.write(content, lock)

    def write(self, content, lock=None):
        """
        Write the supplied content to the html file.
        """
        # Make sure the destination directory exists, if it already does do nothing. If it does not exist try to create
        # it, but include a try statement because it might get created by another process.
        destination = self.path()
        if lock: # Lock is not supplied or needed with build function is called from the liveserver
            with lock:
                if not os.path.exists(destination):
                    os.makedirs(destination)
        else:
            if not os.path.exists(destination):
                os.makedirs(destination)

        # Write the file
        with open(self.url(), 'w') as fid:
            log.debug('Creating {}'.format(self.url()))
            fid.write(content.encode('utf-8'))

    def url(self, parent=None):
        """
        Return the url to the page to be created.
        """
        path = self.path()
        if parent:
            path = os.path.relpath(parent.path(), path)
        return os.path.join(path, 'index.html')
