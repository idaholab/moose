import os, re, urllib, urlparse
from ..slidesets import RemarkSlideSet
from FactorySystem import InputParameters
from utils import colorText

##
# Class for reading django-wiki content
class DjangoWikiSet(RemarkSlideSet):

  ##
  # Valid parameters for the WikiSet class
  @staticmethod
  def validParams():
    params = RemarkSlideSet.validParams()
    params.addRequiredParam('wiki', 'The mooseframework.org wiki site to extract')
    params.addParam('url', 'http://www.mooseframework.org/', 'The location of the Django wiki')
    params.addParam('auto_insert_moose_wiki', True, 'When true links to other moose wiki content is automatically inserted')
    params.addParam('insert_wiki_link', True, 'When auto linking wiki pages place the link at with the page before the inserted content')
    params.addParam('insert_wiki_as_subpage', True, 'When auto linking wiki pages they are inserted with a lower title level')
    params.addParam('use_wiki_title', True, 'When true the title from the wiki page is used as the title')
    params.addParam('use_wiki_title_for_heading', False, 'When true if the first slide does not contain a heading the wiki title is used')
    params.addPrivateParam('article_title', None) # A parameter for storing the wiki article title
    return params


  ##
  # Constructor
  def __init__(self, name, params, **kwargs):
    RemarkSlideSet.__init__(self, name, params, slide_type='DjangoWikiSlide')

    # Define the wiki page
    self.__url = self.getParam('url')

    # Storage for image information, used to re-format the images to normal wiki
    self.images = dict()


  ##
  # Read the raw wiki
  def read(self):

    # Pull the wiki content from mooseframework.org
    page = urlparse.urljoin(self.__url, os.path.join('wiki', self.getParam('wiki')))
    raw = self.__extractMarkdownFromWiki(page)

    # Search for wiki content links
    if self.getParam('auto_insert_moose_wiki'):
      regex = r'(\[.*?\])\((/wiki/.*?)\)'
      raw = re.sub(regex, self.__subLinkedWikiContent, raw)

    # Extract image settings
    raw = re.sub(r'(?<![^\s.])\s*\[\]\(\s*image\s*:([0-9]*)\s*(.*?)\)', self.__subImageSettings, raw)

    # Extract other RemarkJS commands
    raw = re.sub(r'(?<![^\s.])(\s*\[\]\((.*?)\))', self.__subRemark, raw)

    # Extract "[TOC]"
    raw = re.sub(r'(\s*\[TOC\])', self.__subTOC, raw)

    # Return the markdown
    return raw


  ##
  # Method for pulling wiki content from mooseframework.org site (private)
  # @param page The complete website to extract wiki content from
  def __extractMarkdownFromWiki(self, page, **kwargs):

    # Read the _source wiki page html (used to get the raw markdown)
    url = urllib.urlopen(os.path.join(page,'_source'))
    content = url.readlines()
    raw = self.__extractHTML(content, 'pre','class="pre-scrollable"')

    # Extract the article title
    article_title = self.__extractHTML(content, 'h1', 'id="article-title"').split('\n')[0].strip()
    self.parameters()['article_title'] = article_title

    # If desired, use the article title for the slide set title
    if self.getParam('use_wiki_title') and not self.isParamValid('title'):
      self.parameters()['title'] = article_title

    # If the first slide does not contain a heading the "use_wiki_title_for_heading" option is true,
    # use the article title for the first heading of the slide(s)
    if self.getParam('use_wiki_title_for_heading'):
      idx = raw.find('[](---)')
      match = re.search(r'^\s*(#+)\s+(.*?)', raw, flags = re.MULTILINE)
      if not match or match.start(0) > idx:
        print '    Adding article title as heading:', article_title
        raw = '# ' + article_title + '\n' + raw

    # By default, auto-linked content are inserted with headings at the second level
    sub_page = kwargs.pop('sub_page', False)
    if sub_page and self.getParam('insert_wiki_as_subpage'):
      raw = re.sub(r'^\s*(#+)\s+', r'#\1 ', raw, flags = re.MULTILINE)
      subtitle = '[](name:' + '-'.join(article_title.lower().split()) + '-subtitle)\n' \
                 '# ' + article_title + '\n' \
                 '\n[](---)\n\n'
      raw = subtitle + raw

    # Update the image map
    self.__buildImageMap(page)

    # Return the markdown content
    return raw


  ##
  # Substitution method for pulling image settings from the wiki content (private)
  # @param The re Match object, see re.sub
  #
  # On the wiki it is possible to include additional image settings as:
  # [](image:12345 width:50px height:42px)
  # This routine pulls these settings out and stores them in a dict for
  # access when the images are re-formed to traditional wiki form.
  def __subImageSettings(self, match):
    id = match.group(1)
    self.images[id]['settings'] = match.group(2)
    return ''


  ##
  # Substitution method for moose wiki content (private)
  # @param The re Match object, see re.sub
  #
  # This method extracts and inserts wiki content from other pages
  # that are linked on the pages being read by this class
  def __subLinkedWikiContent(self, match):
    page = self.__url + match.group(2)
    content = ''

    if self.getParam('insert_wiki_link'):
      content += match.group(1) + '(' + page + ')\n'

    content += '\n\n---\n\n'
    content += self.__extractMarkdownFromWiki(page, sub_page = True)
    return content


  ##
  # Substitution function for enabling table of contents via [TOC] in wiki content (private)
  # @param The re Match object, see re.sub
  def __subTOC(self, match):
    return ''
    self.parameters()['contents'] = True


  ##
  # Substitution function for extracting Remark properties and commands (private)
  # @param The re Match object, see re.sub
  def __subRemark(self, match):

    # Do not parse comments (i.e., if the string starts with ???), these are parsed at the slide level
    s = match.group(2).strip()
    if s.startswith('???'):
      return match.group(0)
    else:
      return '\n\n' + match.group(2).strip()


  ##
  # Creates an image map from the id to the name and url (private)
  # @param page The complete wiki site address to build image map from
  def __buildImageMap(self, page):

    # Read the wiki content
    url = urllib.urlopen(page)
    wiki = url.read()

    # Read the image information
    url = urllib.urlopen(os.path.join(page, '_plugin/images'))
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
        match = re.search(regex, wiki)
        link = None # case when image is not used
        if match:
          link = urlparse.urljoin(self.__url, match.group(1))

        # Update the name, link lists
        names.append(name)
        links.append(link)

    # Construct the image id to name,url map
    for i in range(len(ids)):
      self.images[ids[i]] = {'name' : names[i], 'url' : links[i], 'settings' : ''}


  ##
  # A method for extracting html blocks (private)
  # @param content The html content to search
  # @param tag The html tag to search for (e.g., div)
  # @param args A list of additional items to search for
  def __extractHTML(self, content, tag, *args):

    # The data to be output
    output = []

    # Beginning/ending tags to search for
    strt = '<' + tag
    stop = '</' + tag + '>'

    # Search each line in the raw wiki html
    store = False # Set to true when the block is located
    for line in content:

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
