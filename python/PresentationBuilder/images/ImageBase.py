import urllib, re
from FactorySystem import MooseObject
import utils

##
# Base class for markdown image maniuplation
class ImageBase(MooseObject):

  @staticmethod
  def validParams():
    params = MooseObject.validParams()
    params.addParam('caption', 'The image caption')
    params.addParam('show_caption', True, 'Toggle the visibility of the caption')
    params.addParam('name', 'The image file name')
    params.addParam('url', 'The image file url')
    params.addParam('download', False, 'Download the image locally')
    params.addPrivateParam('markdown') # The raw markdown source for this image

    params.addParam('div-center', True, 'Wrap html figure in centered div')
    params.addParam('align', 'The image horizontal alignment')
    params.addParam('width', 'The image width')
    params.addParam('height', 'The image height')
    params.addParam('style', 'The img style property')
    params.addParam('vertical-align', 'The vertical alignment of the image')
    params.addParam('text-align', 'Text alignment of image')
    params.addParam('caption', 'The caption for the image')
    params.addParamsToGroup('html', ['align', 'width', 'height', 'vertical-align', 'text-align'])

    return params

  ##
  # Constructor
  def __init__(self, name, params):
    MooseObject.__init__(self, name, params)
    self.parent = self.getParam('_parent')

    # Set download flag (default is true)
    self._download = True
    if self.isParamValid('download'):
      self.__download = self.parameters()['download']

    # Images are formated has html, if backticks where added to equations then need to be removed
    if self.isParamValid('caption'):
      self._pars['caption'] = re.sub(r'`(\$.*?\$)`', r'\1', self._pars['caption'])

  ##
  # Perform the matching
  # @param markdown The raw markdown to parse
  # @return A list of dictionaries
  #
  # The dicts must contains:
  #  raw, url, caption, settings
  def match(self, markdown):
    return []

  ##
  # Return the image html for replacing the markdown image syntax
  def html(self):

    # Get the name and url
    name = self.name()
    url = self.getParam('url')

    # Do not download the image
    if self.__download:
      urllib.urlretrieve(url, name)
      img_name = name
    else:
      img_name = url

    # Flag for wrapping in div
    div = self.getParam('div-center')
    if isinstance(div, str):
      div = utils.str2bool(div)

    # Create the html <img> block
    img = '\n\n'
    if div:
      img += '<div style="text-align:center;">\n'
    img += '<figure style="float:left">\n'
    img += '  <a href="' + url + '">\n'
    img += '    <img src="' + img_name + '"'

    if self.isParamValid('style'):
      style = self.getParam('style')
    else:
      style = ''
      for prop in self._pars.groupKeys('html'):
        if self.isParamValid(prop):
          style += prop + ':' + self._pars[prop] + ';'

    if style:
      img += ' style="'+style+'"'
    img += '/>\n  </a>\n'

    # Create a table that contains the image and caption (if desired)
    if self._pars['show_caption'] and self.isParamValid('caption'):
      img += '  <figcaption>\n'
      img += '    ' + self._pars['caption']
      img += '\n  </figcaption>\n'

    img += '</figure>\n'

    if div:
      img += '</div>'

    # Return the complete html
    return img
