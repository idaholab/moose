import os
from src.images import ImageBase

##
# Image class for standard markdown image blocks
class MarkdownImage(ImageBase):

  re = r'\!\[(.*?)\]\((.*)\)'

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
  # Substitution regex
  def sub(self):
    return r'\!\[' + self._pars['caption'] + '\]\(' + self._pars['name'] + '\)'
