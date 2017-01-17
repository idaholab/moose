import sys
import os
import re
import io
import traceback
from pybtex.plugin import find_plugin
from pybtex.database import BibliographyData, parse_file

import MooseDocs
from MooseCommonExtension import MooseCommonExtension
from markdown.preprocessors import Preprocessor
from markdown.util import etree

import logging
log = logging.getLogger(__name__)

class MooseBibtex(MooseCommonExtension, Preprocessor):
  """
  Creates per-page bibliographies using latex syntax.
  """

  RE_BIBLIOGRAPHY = r'(?<!`)\\bibliography\{(.*?)\}'
  RE_STYLE = r'(?<!`)\\bibliographystyle\{(.*?)\}'
  RE_CITE = r'(?<!`)\\(?P<cmd>cite|citet|citep)\{(?P<key>.*?)\}'

  def __init__(self, markdown_instance=None, **kwargs):
    MooseCommonExtension.__init__(self, **kwargs),
    Preprocessor.__init__(self, markdown_instance)

  def run(self, lines):
    """
    Create a bibliography from cite commands.
    """

    # Join the content to enable regex searches throughout entire text
    content = '\n'.join(lines)

    # Build the database of bibtex data
    self._citations = []              # member b/c it is used in substitution function
    self._bibtex = BibliographyData() # ""
    bibfiles = []
    match = re.search(self.RE_BIBLIOGRAPHY, content)
    if match:
      bib_string = match.group(0)
      for bfile in match.group(1).split(','):
        try:
          bibfiles.append(MooseDocs.abspath(bfile.strip()))
          data = parse_file(bibfiles[-1])
        except Exception as e:
          log.error('Failed to parse bibtex file: {}'.format(bfile.strip()))
          traceback.print_exc(e)
          return lines
        self._bibtex.add_entries(data.entries.iteritems())
    else:
      return lines

    # Determine the style
    match = re.search(self.RE_STYLE, content)
    if match:
      content = content.replace(match.group(0), '')
      try:
        style = find_plugin('pybtex.style.formatting', match.group(1))
      except:
        log.error('Unknown bibliography style "{}"'.format(match.group(1)))
        return lines

    else:
      style = find_plugin('pybtex.style.formatting', 'plain')

    # Replace citations with author date, as an anchor
    content = re.sub(self.RE_CITE, self.authors, content)

    # Create html bibliography
    if self._citations:

      # Generate formatted html using pybtex
      formatted_bibliography = style().format_bibliography(self._bibtex, self._citations)
      backend = find_plugin('pybtex.backends', 'html')
      stream = io.StringIO()
      backend().write_to_stream(formatted_bibliography, stream)

      # Strip the bib items from the formatted html
      html = re.findall(r'\<dd\>(.*?)\</dd\>', stream.getvalue(), flags=re.MULTILINE|re.DOTALL)

      # Produces an ordered list with anchors to the citations
      output = u'<ol class="moose-bibliography" data-moose-bibfiles="{}">\n'.format(str(bibfiles))
      for i, item in enumerate(html):
        output += u'<li name="{}">{}</li>\n'.format(self._citations[i], item)
      output += u'</ol>\n'
      content = re.sub(self.RE_BIBLIOGRAPHY, self.markdown.htmlStash.store(output, safe=True), content)

    return content.split('\n')

  def authors(self, match):
    """
    Return the author(s) citation for text, linked to bibliography.
    """
    cmd = match.group('cmd')
    key = match.group('key')
    tex = '\\%s{%s}' % (cmd, key)

    if key in self._bibtex.entries:
      self._citations.append(key)
      entry = self._bibtex.entries[key]
      a = entry.persons['author']
      n = len(a)
      if n > 2:
        author = '{} et al.'.format(' '.join(a[0].last_names))
      elif n == 2:
        a0 = ' '.join(a[0].last_names)
        a1 = ' '.join(a[1].last_names)
        author = '{} and {}'.format(a0, a1)
      else:
        author = ' '.join(a[0].last_names)

      if cmd == 'citep':
        a = '<a href="#{}" data-moose-cite="{}">{}, {}</a>'.format(key, tex, author, entry.fields['year'])
        return '({})'.format(self.markdown.htmlStash.store(a, safe=True))
      else:
        a = '<a href="#{}" data-moose-cite="{}">{} ({})</a>'.format(key, tex, author, entry.fields['year'])
        return self.markdown.htmlStash.store(a, safe=True)
