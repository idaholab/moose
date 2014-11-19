import os, re, urllib
from src.images import DjangoWikiImage
from src.slides import RemarkSlide

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

  def parse(self, markdown):
    markdown = RemarkSlide.parse(self, markdown)

    # Replace special characters
    for item in self.replace:
      markdown = markdown.replace(item[0], item[1])

    # In-line equations
    pattern = re.compile('\$\$(.*?)\$\$')
    for m in pattern.finditer(markdown):
      markdown = markdown.replace(m.group(0), '`$ ' + m.group(1) + ' $`')

    # Block equations
    pattern = re.compile(r'\$\$\$(.*?)\$\$\$', re.S)
    for m in pattern.finditer(markdown):
      markdown = markdown.replace(m.group(0), '\n`$$ ' + m.group(1) + ' $$`')

    return markdown
