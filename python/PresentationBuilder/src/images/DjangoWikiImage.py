import re, urllib

from src.images import ImageBase


##
# Image class for DjangoWikiSlide class
class DjangoWikiImage(ImageBase):

  re = r'\s*\[image:([0-9]*)(.*?)\]\s*\n\s{4,}(.*?)\n'

  @staticmethod
  def validParams():
    params = ImageBase.validParams()
    params['download'] = True # by default download the image
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

    # Store the caption, if it is not provided in the parameters
    if not self.isParamValid('caption'):
      self._pars['caption'] = self.match.group(3).replace('\r', '')

    # Grab the image alignment if it is not provided
    if not self.isParamValid('align'):
      align = re.search(r'align:\s*(.*)', self.match.group(2))
      if align:
        self._pars['align'] = align.group(1)

    # Set the name and url parameters
    name, url = self._image_map[self.name()]
    if not self.isParamValid('name'):
      self._pars['name'] = name
    if not self.isParamValid('url'):
      self._pars['url'] = url

  ##
  # Substitution regex
  def sub(self):
    return r'\[image:' + str(self._id) + '(.*?)\]\s*\n\s{4,}(.*?)\n'
