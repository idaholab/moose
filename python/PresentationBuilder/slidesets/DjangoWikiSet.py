import os, re, urllib, urlparse
from ..slidesets import SlideSet
from FactorySystem import InputParameters

##
# Class for reading django-wiki content
class DjangoWikiSet(SlideSet):

  ##
  # Valid parameters for the WikiSet class
  @staticmethod
  def validParams():
    params = SlideSet.validParams()
    params.addRequiredParam('wiki', 'The mooseframework.org wiki site to extract')
    params.addParam('url', 'http://www.mooseframework.org/', 'The location of the Django wiki')
    return params


  ##
  # Constructor
  def __init__(self, name, params, **kwargs):
    SlideSet.__init__(self, name, params, slide_type='DjangoWikiSlide')

    # Define the wiki page
    self._url = self.getParam('url')
    self._page = urlparse.urljoin(self._url, os.path.join('wiki', self.getParam('wiki')))

    # Read the _source wiki page html (used to get the raw markdown)
    url = urllib.urlopen(os.path.join(self._page,'_source'))
    self.source = url.readlines()

    # Read the actual wiki (used for image extraction)
    url = urllib.urlopen(self._page)
    self.wiki = url.read()

    # Build image map
    self.images = dict()
    self._buildImageMap()

    # Storage for comments
    self._comments = []

  ##
  # Read the raw wiki
  def read(self):
    raw = self._extractHTML('pre','class="pre-scrollable"')

    # Extract comments
    raw = re.sub(r'(?<![^\s.])(\s*\[\]\(\?\?\?\s*(.*?)\))', self._storeComment, raw)

    # Extract other RemarkJS commands
    raw = re.sub(r'(?<![^\s.])(\s*\[\]\((.*?)\))', self._applyRemark, raw)

    # Add the comments at the end
    if self._comments:
      raw += '\n???\n'
      for c in self._comments:
        raw += c

    # Return the markdown
    return raw

  ##
  # Substitution function for extracting Remark properties and commands (private)
  def _applyRemark(self, match):
    return '\n\n' + match.group(2).strip()

  ##
  # Substituion function for extracting Remark comments (private)
  def _storeComment(self, match):
    self._comments.append(match.group(2).strip())
    return ''

  ##
  # Creates an image map from the id to the name and url (private)
  def _buildImageMap(self):

    # Read the image information
    url = urllib.urlopen(os.path.join(self._page, '_plugin/images'))
    raw = url.read()

    # Extract wiki image ids
    ids = []
    pattern = re.compile(r'\[image:([0-9]*)(.*?)\]')
    for m in pattern.finditer(raw):
      ids.append(m.group(1))

    # Extract file names and url to image
    names = []
    links = []
    pattern = re.compile(r'alt="(.*?)"')
    for m in pattern.finditer(raw):

      name = m.group(1)
      if name not in names:
        # Locate image URL
        regex = 'href=\"(/static/media/wiki/images/' + '.*' + name + ')">.*'
        match = re.search(regex, self.wiki)
        link = None # case when image is not used
        if match:
          link = urlparse.urljoin(self._url, match.group(1))

        # Update the name, link lists
        names.append(name)
        links.append(link)

    # Construct the image id to name,url map
    for i in range(len(ids)):
      self.images[ids[i]] = [names[i], links[i]]

  ##
  # A method for extracting html blocks (private)
  # @param tag The html tag to search for (e.g., div)
  # @param args A list of additional items to search for
  def _extractHTML(self, tag, *args):

    # The data to be output
    output = []

    # Beginning/ending tags to search for
    strt = '<' + tag
    stop = '</' + tag + '>'

    # Search each line in the raw wiki html
    store = False # Set to true when the block is located
    for line in self.source:

      # The desired block was located
      if strt in line and all(arg in line for arg in args):
        store = True

      # The block has reached the end
      elif store and stop in line:
        store = False
        break;

      # Store the data
      elif store:
        output.append(line)

    # Done
    return ''.join(output)
