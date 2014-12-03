import os, re, urllib
from ..images import DjangoWikiImage
from ..slides import RemarkSlide

##
# A slide for wiki content from a Djanjo Wiki (https://github.com/django-wiki/django-wiki)
class DjangoWikiSlide(RemarkSlide):

  @staticmethod
  def validParams():
    params = RemarkSlide.validParams()
    return params

  # When reading the markdown these replacements are made
  replace = [('&amp;', '&'), ('&lt;', '<'), ('&gt;', '>'), ('\r\n', '\n')]

  ##
  # Constructor
  # @param id The numeric slide id
  # @param markdown The raw markdown for this slide
  # @param kwargs Optional key, value pairs
  def __init__(self, name, params):
    RemarkSlide.__init__(self, name, params, image_type='DjangoWikiImage')

    # Storage for comments
    self._comments = []

  def parse(self, markdown):
    markdown = RemarkSlide.parse(self, markdown)

    # Replace special characters
    for item in self.replace:
      markdown = markdown.replace(item[0], item[1])

    # Equations
    pattern = re.compile('(\${1,})(.*?)\${1,}', re.S)
    for m in pattern.finditer(markdown):

      # Inline
      if m.group(1) == '$$':
        markdown = markdown.replace(m.group(0), '`$ ' + m.group(2) + ' $`')

      elif m.group(1) == '$$$':
        markdown = markdown.replace(m.group(0), '`$$ ' + m.group(2) + ' $$`')

      else:
        print 'ERROR parsing equation on slide', self.name()
        print '  ', m.group(2)

    # Extract comments
    markdown = re.sub(r'(?<![^\s.])(\s*\[\]\(\?\?\?\s*(.*?)\))', self._storeComment, markdown)

    # Add the comments at the end
    if self._comments:
      prefix = '\n'
      if len(self._comments) > 1:
        prefix = '\n- '

      markdown += '\n???\n'
      for c in self._comments:
        markdown += prefix + c

    # Return the markdown
    return markdown

  ##
  # Substitution function for extracting Remark comments (private)
  def _storeComment(self, match):
    print match.group(2).strip()
    self._comments.append(match.group(2).strip())
    return ''
