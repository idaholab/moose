import re, urllib
from ..images import ImageBase

##
# Image class for DjangoWikiSlide class
class DjangoWikiImage(ImageBase):

  @staticmethod
  def validParams():
    params = ImageBase.validParams()
    return params

  @staticmethod
  def extractName(match):
    return match.group(1).strip()

  ##
  # Constructor
  # @param match The result of the regex search
  def __init__(self, name, params):
    ImageBase.__init__(self, name, params)

    # Get a reference to the image map contained in DjangoSlideSet
    image_settings = self.parent.parent.images[name]

    # Set the name and url parameters
    if not self.isParamValid('name'):
      self.parameters()['name'] = name
    if not self.isParamValid('url'):
      self.parameters()['url'] = image_settings['url']

    # Apply the settings from the DjangoSlideSet
    if image_settings['settings']:
      print ' '*8 + 'Appling image settings from wiki'
    for pair in image_settings['settings'].split():
      k,v = pair.strip().split(':')
      if k in params:
        params[k] = v

  ##
  # Performs the regex matching for Django images
  @staticmethod
  def match(markdown):

    # This list to be output
    output = []

    # A list of ids to avoid outputting the same image twice
    ids = []

    # Caption
    pattern = re.compile(r'\s*\[image:([0-9]*)(.*)\]\s*\n\s{4,}(.*?)\n')
    for m in pattern.finditer(markdown):
      ids.append(m.group(1))
      output.append({'markdown' : m.group(0), \
                     'name' : m.group(1), \
                     'caption' : m.group(3), \
                     'url' : None, \
                     'settings' : m.group(2)})

    # No caption
    pattern = re.compile(r'\s*\[image:([0-9]*)(.*)\]\s*\n')
    for m in pattern.finditer(markdown):
      id = m.group(1)
      if id not in ids:
        output.append({'markdown' : m.group(0), \
                       'name' : m.group(1), \
                       'caption' : None, \
                       'url' : None, \
                       'settings' : m.group(2)})

    # Return the list
    return output
