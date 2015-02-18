import os, re
from ..images import ImageBase

##
# Image class for standard markdown image blocks
class MarkdownImage(ImageBase):

  @staticmethod
  def validParams():
    params = ImageBase.validParams()
    return params

  @staticmethod
  def extractName(match):
    f, e = os.path.splitext(match.group(2))
    return f.split('/')[-1]

  ##
  # Constructor
  def __init__(self, name, params):
    ImageBase.__init__(self, name, params)

    if not self.isParamValid('name'):
      self._pars['name'] = self.match.group(2)

    if not self.isParamValid('caption'):
      self._pars['caption'] = self.match.group(1)

    if not self.isParamValid('url'):
      self._pars['url'] = self.match.group(2)

  ##
  #
  @staticmethod
  def match(markdown):

    # List of match iterators to return
    m = dict()

    pattern = re.compile(r'\!\[(.*?)\]\((.*)\)')
    for item in pattern.finditer(markdown):
      m[item.group(1)] = ImageBase.seperateImageOptions(item.group(2))

    # Return the list
    return m

  ##
  # Substitution regex
  def sub(self):
    return r'\!\[' + self._pars['caption'] + '\]\(' + self._pars['name'] + '\)'
