import os
import copy
import bs4
import jinja2
import logging
log = logging.getLogger(__name__)

import MooseDocs
from MooseDocsMarkdownNodeBase import MooseDocsMarkdownNodeBase

class MooseDocsMarkdownNode(MooseDocsMarkdownNodeBase):
    """
    Node for converting markdown to html for use by the build.py script.
    """
    def __init__(self, **kwargs):
        super(MooseDocsMarkdownNode, self).__init__(**kwargs)

    def code(self, repo_url):
        """
        Return the GitHub/GitLab address for editing the markdown file.

        Args:
          repo_url[str]: Web address to use as the base for creating the edit link
        """
        info = []
        for key, syntax in self._syntax.iteritems():
            for obj in syntax.objects().itervalues():
                if obj.name == self.name():
                    info.append(obj)
            for obj in syntax.actions().itervalues():
                if obj.name == self.name():
                    info.append(obj)

        output = []
        for obj in info:
            for filename in obj.code:
                rel_filename = MooseDocs.relpath(filename)
                output.append( (os.path.basename(rel_filename), os.path.join(repo_url, 'blob', 'master', rel_filename)) )

        return output

    def doxygen(self):
        """
        Return the url to the markdown file for this object.
        """
        # Build a complete list of objects
        for syntax in self._syntax.itervalues():
            for obj in syntax.objects().itervalues():
                if obj.name == self.name():
                    return syntax.doxygen(obj.name)

    def editMarkdown(self, repo_url):
        """
        Return the url to the markdown file for this object.
        """
        return os.path.join(repo_url, 'edit', 'devel', MooseDocs.relpath(self.source()))
