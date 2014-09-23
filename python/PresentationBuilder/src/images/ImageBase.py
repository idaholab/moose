from FactorySystem import MooseObject

##
# Base class for markdown image maniuplation
class ImageBase(MooseObject):

  re = ''

  @staticmethod
  def validParams():
    params = MooseObject.validParams()
    params.addParam('caption', 'The image caption')
    params.addParam('show_caption', True, 'Toggle the visibility of the caption')
    params.addParam('name', 'The image file name')
    params.addParam('url', 'The image file url')
    params.addParam('download', False, 'Download the image locally')

    params.addParam('align', 'The image horizontal alignment')
    params.addParam('width', 'The image width')
    params.addParam('height', 'The image height')
    params.addParamsToGroup('html', ['align', 'width', 'height', 'data-rotate'])

    return params

  @staticmethod
  def extractName(match):
    return None

  ##
  # Constructor
  def __init__(self, name, params):
    MooseObject.__init__(self, name, params)
    self.parent = self.getParam('_parent')
    self.match = self.getParam('_match')

    # Store the complete markdown image text
    self._raw = self.match.group(0)

  ##
  # Return the regex for performing image substitution (virtual)
  def sub(self):
    return ''

  ##
  # Return the image html for replacing the markdown image syntax
  def html(self):

    # Get the name and url
    name = self.getParam('name')
    url = self.getParam('url')

    # Download the image
    if self._pars['download']:
      urllib.urlretrieve(url, name)
      img_name = name
    else:
      img_name = url

    # Create the html <img> block
    img = '    <a href="' + url + '">\n'
    img += '      <img src="' + img_name + '"'

    for prop in self._pars.groupKeys('html'):
      if self.isParamValid(prop):
        img += ' ' + prop + '="' + self._pars[prop] + '"'

    img += '/>\n'
    img += '    </a>\n'

    # Create a table that contains the image and caption (if desired)
    html = '<table'
    for prop in self._pars.groupKeys('html'):
      if self.isParamValid(prop):
        html += ' ' + prop + '="' + self._pars[prop] + '"'
    html += '>\n'
    html += '  <tr><td>\n' + img + '  </tr></td>\n'

    if self._pars['show_caption']:
      html += '  <tr><td>' + self._pars['caption'] + '</tr></td>\n'
    html += '</table>'

    # Return the complete html
    return html
