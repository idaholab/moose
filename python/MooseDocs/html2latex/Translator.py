import re
import bs4
import logging
log = logging.getLogger(__name__)
from Extension import Extension
from ElementStorage import ElementStorage
import elements

class Translator(object):
  """
  Class for converting from html to latex.

  Args:
    extensions[list]: List of Extension objects. If empty the BasicExtension will be used.
  """
  def __init__(self, unknown=elements.unknown(), extensions=[]):

    #: BlockElement objects
    self.elements = ElementStorage(etype=elements.Element)

    #: Unknown tag conversion
    self.unknown = unknown

    # Add extensions
    for e in extensions:
      if isinstance(e, type):
        obj = e()
      else:
        obj = e

      if not isinstance(obj, Extension):
        raise Exception('Invalid extension type or instance provided, expected {} but was given {}.'.format('Extension', type(e)))
      obj.extend(self)


  def convert(self, html):
    """
    Convert supplied html to latex.

    Args:
      html[str]: The raw html to convert to latex.
    """

    # The html parser attempts to match < > even when they are inside code blocks
    def sub(match):
      return '<code{}>{}</code>'.format(match.group(1), match.group(2).replace('<', '##LESSTHAN##').replace('>', '##GREATERTHAN##').replace('&lt;','##LESSTHAN##').replace('&gt;','##GREATERTHAN##'))
    html = re.sub(r'<code(.*?)>(.*?)</code>', sub, html, flags=re.MULTILINE|re.DOTALL)

    def html2latex(input):
      n_tags = 0
      soup = bs4.BeautifulSoup(input, "html.parser")
      output = []
      for child in soup.children:
        if isinstance(child, bs4.element.Tag):
          n_tags += 1
          result = self._convertTag(child)
          if result:
            output += result
          else:
            output += unicode(child)
        else:
          output += unicode(child)

      tex = ''.join(output)
      return (tex, n_tags)

    tex = html
    old_n_tags = 0
    while (True):
      tex, n_tags = html2latex(tex)
      if n_tags == old_n_tags:
        break
      old_n_tags = n_tags
    #if n_tags > 0:
    #    print 'Failed to convert all html tags.'


    output = []
    soup = bs4.BeautifulSoup(tex, "html.parser")
    for child in soup.children:
      if isinstance(child, bs4.element.Tag):
        log.error('Failed to convert tag: {}'.format(child.name))
        output += self.unknown.convert(child, self.unknown.content(child))
      else:
        output += unicode(child)
    tex = ''.join(output)

    return tex.replace('##LESSTHAN##', '<').replace('##GREATERTHAN##', '>')

  def _convertTag(self, tag):
    """
    Convert tag to latex.

    Args:
      tag[bs4.element.Tag]: The tag element to convert.
    """

    for obj in self.elements:
      if obj.test(tag):
        #@todo check return type of convert(), expects a string
        return obj.prefix(tag) + obj.convert(tag, obj.content(tag))

    return ''

  def preamble(self):
    """
    Return the latex preamble content.
    """

    output = []

    for obj in self.elements:
      preamble = obj.preamble()
      if not isinstance(preamble, list):
        log.error("The preamble method of {} must return a list.".format(obj.__class__.__name__))
      for p in preamble:
        if p not in output:
          output.append(p)

    return '\n'.join(output)
