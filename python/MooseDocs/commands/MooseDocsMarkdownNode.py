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
    def __init__(self, navigation=None, **kwargs):
        super(MooseDocsMarkdownNode, self).__init__(**kwargs)

        self._template_args['navigation'] = navigation

    def finalize(self, soup):
        """
        Adds markdown link creation and table-of-contents handling.
        """
        soup = super(MooseDocsMarkdownNode, self).finalize(soup)

        def finder(node, desired, pages):
            """
            Locate nodes for the 'desired' filename
            """
            if node.source() and node.source().endswith(desired):
                pages.append(node)
            for child in node:
                finder(child, desired, pages)
            return pages

        # Loop over <a> tags and update links containing .md to point to .html
        for link in soup('a'):
            href = link.get('href')
            if href and (not href.startswith('http')) and href.endswith('.md'):
                found = []
                finder(self.root(), href, found)

                # Error if file not found or if multiple files found
                if not found:
                    #TODO: convert to error when MOOSE is clean
                    log.warning('Failed to locate page for markdown file {} in {}'.format(href, self.source()))
                    link['class'] = 'moose-bad-link'
                    continue

                elif len(found) > 1:
                    msg = 'Found multiple pages matching the supplied markdown file {} in {}:'.format(href, self.source())
                    for f in found:
                        msg += '\n    {}'.format(f.source())
                    log.error(msg)

                # Update the link with the located page
                url = self.relpath(found[0].url())
                #log.debug('Converting link: {} --> {}'.format(href, url))
                link['href'] = url

        # Add materialize sections for table-of-contents
        div = soup.find('div', id='moose-markdown-content')
        if div:
            current = div
            for tag in div.contents:
                if isinstance(tag, bs4.element.Tag):
                    if tag.name == 'h2':
                        current = soup.new_tag('div', id=tag.get('id', '#'))
                        current.attrs['class'] = "section scrollspy"

                    if current != tag.parent:
                        tag.wrap(current)

        return soup

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

    def contents(self, level='h2'):
        """
        Return the table of contents.
        """
        soup = bs4.BeautifulSoup(self.content(), 'html.parser')
        for tag in soup.find_all(level):
            if 'id' in tag.attrs:
                yield (tag.contents[0], tag.attrs['id'])
