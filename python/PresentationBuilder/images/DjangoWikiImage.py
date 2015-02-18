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

    # Store the Django wiki image id
    self._id = int(name)

    # Get a reference to the image map contained in DjangoSlideSet
    self._image_map = self.parent.parent.images

    # Set the name and url parameters
    name, url = self._image_map[self.name()]
    if not self.isParamValid('name'):
      self._pars['name'] = name
    if not self.isParamValid('url'):
      self._pars['url'] = url

  ##
  # Performs the regex matching for Django images
  @staticmethod
  def match(markdown):

    # List of match iterators to return
    m = dict()

    # Caption
    pattern = re.compile(r'\s*\[image:([0-9]*)(.*)\]\s*\n\s{4,}(.*?)\n')
    for item in pattern.finditer(markdown):
      id = item.group(1)
      m[id] = ImageBase.seperateImageOptions(item.group(2))
      m[id].update({'caption' : item.group(3)})

    # No caption
    pattern = re.compile(r'\s*\[image:([0-9]*)(.*)\]\s*\n')
    for item in pattern.finditer(markdown):
      id = item.group(1)
      m[id] = ImageBase.seperateImageOptions(item.group(2))

    # Return the list
    return m


  ##
  # Substitution regex
  def sub(self):

    if self.isParamValid('caption'):
      return r'\[image:' + str(self._id) + '(.*?)\]\s*\n\s{4,}(.*?)\n'
    else:
      return r'\[image:' + str(self._id) + '(.*?)\]\s*\n'
