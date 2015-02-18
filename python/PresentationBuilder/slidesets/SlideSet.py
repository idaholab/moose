# Load python packages
import re, sys, math
from collections import OrderedDict

# Load Moose packages
from FactorySystem import MooseObject
from ..slides import *

##
# Base class for markdown slide generation
class SlideSet(MooseObject):

  ##
  # Defines the available properties for the SlideSet base class
  @staticmethod
  def validParams():
    params = MooseObject.validParams()
    params.addRequiredParam('type', 'The type of slide set to create')
    params.addParam('title', 'The title of the slide set, if this exists a title slide will be injected')
    params.addParam('active', 'A list of ordered slide names to output, if blank all slides are output')
    params.addParam('inactive', 'A list of slide names to exclude from output')
    params.addParam('contents', False, 'Include table of contents slide')
    params.addParam('contents_title', 'The table-of-contents heading for this slide set')
    params.addParam('contents_level', 1, 'The heading level to include in the contents')
    params.addParam('contents_items_per_slide', 11, 'The number of contents items to include on a page')
    params.addParam('show_in_contents', True, 'Toggle if slide set content appears in the table-of-contents')
    params.addParam('style', 'The CSS style sheet to utilize for this slide set')
    params.addParam('non_ascii_warn', True, 'Produce warning if non-ascii characters are located')

    # Create the common parameters from RemarkSlide 'properties' group
    slide_params = RemarkSlide.validParams()
    for key in slide_params.groupKeys('properties'):
      params.addParam(key, slide_params.getDescription(key))
    params.addParamsToGroup('properties', slide_params.groupKeys('properties'))

    return params

  ##
  # Constructor
  # @param name The name of the object
  # @param params The InputParameters for the object being created
  # @param kwars Optional key, value pairings
  #
  # Optional key, value pairs:
  #   slide_type = <str>
  #   The name of the Slide class to build, by default 'Slide' is used
  def __init__(self, name, params, **kwargs):
    MooseObject.__init__(self, name, params)

    # Get a reference to the factory
    self._warehouse = self.getParam('_warehouse')
    self._factory = self.getParam('_factory')
    self._parser = self.getParam('_parser')
    self._root = self.getParam('_root')

    # Set the slide type
    self._slide_type = kwargs.pop('slide_type', 'RemarkSlide')

    # Initialize storage for the generate slides, use an ordered dict to maintain slide ordering
    self._slides = dict()
    self._slide_order = []
    self._content_slides = None # name(s) of contents slides
    self._title_slide = None # name of title slides

    # Count the number of slides, used for assigning default name
    self._count = 0

    # Storage for links
    self._links = []

    # Print a message
    print '  CREATED:', name

  ##
  # Return the active slide names
  def activeSlides(self):

    # By default return all slides, in order
    slides = self._slide_order

    # Limit the slides to the active list
    if self.isParamValid('active'):
      slides = self.getParam('active').split()
      for name in slides:
        if name not in self._slides:
          slides.remove(name)
          print 'WARNING: Slide name ' + name + ' is unknown'

      # Insert the contents and title slides to the active list
      if self._content_slides != None:
        slides = self._content_slides + slides
      if self._title_slide != None:
        slides.insert(0, self._title_slide)

    # Remove inactive slides
    if self.isParamValid('inactive'):
      for name in self.getParam('inactive').split():
        if name in slides:
          slides.remove(name)

    return slides

  ##
  # The method that creates/retrieves the markdown (virtual)
  def read(self):
    return ''


  ##
  # Initial setup creates the title and contents slide, if desired
  def setup(self):


    # Apply title slide
    if self.isParamValid('title'):
      self._title_slide = self.name() + '-title'
      slide = self._createSlide('# ' + self.getParam('title') + '\n', show_in_contents=False, title=True,
                                name=self._title_slide)


  ##
  # Creates slides from the raw_markdown input
  # @param raw_markdown The RemarkJS markdown to parse into slides
  def build(self, raw_markdown):

    # Do nothing if the raw_markdown is empty
    if not raw_markdown:
      return

    # Test if raw markdown contains non-ascii
    if self.getParam('non_ascii_warn'):
      non_ascii = []
      lines = raw_markdown.split('\n')
      for line in lines:
        try:
          line.encode('ascii')
        except UnicodeDecodeError:
          non_ascii.append(line)

      if len(non_ascii) > 0:
        print 'WARNING: Slideset ' + self.name() + ' contains non-ascii characters'
        for item in non_ascii:
          print item
        print '\n'

    # Extract links
    match = re.findall('^(\s*\[(.*)\]:(.*))', raw_markdown, re.MULTILINE)
    for m in match:
      self._links.append(m[0])

    # Separate the individual slides
    raw_slides = re.split(r'\n---', raw_markdown)

    # Create/store the slides
    for raw in raw_slides:
      self._createSlide(raw)


  ##
  # Subsitituion function for capturing markdown links
  def _captureLinks(self, match):
    print match.group(0)
    self._links.append(match.group(0))
    return ''


  ##
  # Create the a slide from raw markdown (private)
  # @param raw The raw markdown to build the slide from
  # @param kwargs Optional key, value pairs
  #
  # Optional Parameters:
  #  name = <str>
  #  The default slide name, it this is not contained the
  #  default name is the assigned to the slide set name with
  #  the slide number. Regardless of the default name, the
  #  slide name will ALWAYS be changed to use the heading
  #
  #  index
  def _createSlide(self, raw, **kwargs):

    # Get the class from the name of the slide
    slide_class = globals()[self._slide_type]

    # Register the slide object to be created
    self._factory.register(slide_class, self._slide_type)

    # Get the Slide name
    name = kwargs.pop('name', slide_class.extractName(raw))
    if name == None:
      name = self.name() + '-' + str(self._count)

    # Indicate that the slide is being created
    print '    SLIDE:', name

    # Get the default input parameters from the slide being created
    params = self._factory.validParams(self._slide_type)
    params.applyParams(self._pars)

    # Apply the common properties from this class
    for key in params.groupKeys('properties'):
      if self.isParamValid(key):
        params[key] = self.getParam(key)

    # Apply the [./Slides] block
    if self._root:
      node = self._root.getNode(self.name()).getNode('Slides')
      if node:
        node = node.getNode(name)
        if node:
          print ' '*6 + 'Apply settings from input file'
          self._parser.extractParams('', params, node)

    # Add the parent and markdown parameters
    params.addPrivateParam('_parent', self)
    params.addPrivateParam('format', self._warehouse.format)
    params.addRequiredParam('markdown', raw, 'The raw markdown to parse for the current slide')

    # Over-ride parameters with optional key, value pairs
    for key, value in kwargs.iteritems():
      params[key] = value

    # Build and store the slide
    slide = self._factory.create(self._slide_type, name, params)
    self._slides[name] = slide

    idx = kwargs.pop('index', None)
    if idx != None:
      self._slide_order.insert(idx, name)
    else:
      self._slide_order.append(name)
    self._count += 1

    # Call the parse method and populate the markdown
    slide._createImages(raw)
    slide.markdown = slide.parse(raw)
    slide._insertImages()


  ##
  # A helper method for extracting the contents entries from slides
  def _extractContents(self):
    contents = []

    # Get the contents from the slides
    for name in self.activeSlides():
      contents += self._slides[name].contents()
    return contents


  ##
  # Initialize contents
  # This creates and inserts the correct number of contents slides
  def initContents(self):

    # Do nothing if the 'contents' flag is not set in the input file
    if not self.getParam('contents'):
      return

    # Extract the contents
    contents = self._extractContents()

    # Determine the number of contents slides needed
    max_per_slide = float(self.getParam('contents_items_per_slide'))
    n = int(math.ceil(len(contents) / max_per_slide))

    # Determine the table of contents header
    if self.isParamValid('contents_title'):
      contents_title = '# ' + self.getParam('contents_title') + '\n'
    elif self.isParamValid('title'):
      contents_title = '# ' + self.getParam('title') + ' Contents\n'
    else:
      contents_title = '# Contents\n'

    # Locate the slide insert location
    if self.name() + '-title' in self._slides:
      idx = 1
    else:
      idx = 0

    # Add the content(s) slides
    self._content_slides = []
    for i in range(n):
      if i > 0:
        contents_title = ''
      self._content_slides.append(self.name() + '-contents-' + str(i))
      self._createSlide(contents_title, name = self._content_slides[i], show_in_contents=False, index=idx)
      idx += 1


  ##
  # Build the table-of-contents
  def contents(self):

    # Do nothing if the 'contents' flag is not set in the input file
    if not self.getParam('contents'):
      return

    # Update the contents object
    contents = self._extractContents()

    # Build the table-of-contents entries
    max_per_slide = int(self.getParam('contents_items_per_slide'))
    lvl = int(self.getParam('contents_level'))
    output = ['']
    count = 0
    page = 0
    for item in contents:
      if item[2] <= lvl:
        title = item[0]         # the heading content
        name = item[1]          # slide name
        indent = 25*(item[2]-1) # heading level indenting
        idx = str(item[3])      # slide index

        #if slide.isParamValid('line-height'):
        #  height = slide.getParam('line-height')
        #else:
        height = '15px'

        # Build a link to the slide, by name
        link = '<a href="#' + name + '">'

        # Create the contents entry
        output[page] += '<p style="line-height:' + height + ';text-align:left;text-indent:' + str(indent) + 'px;">' + link + title + '</a>'
        output[page] += '<span style="float:right;">' + link + idx + '</a>'
        output[page] += '</span></p>\n'

        count += 1
        if (count > max_per_slide):
          count = 0
          page += 1
          output.append('')

    for i in range(len(output)):
      if not output[i]:
        print 'Warning: Contents for slide set, ' + self.name() + ', are empty.'
        break

      contents_name = self.name() + '-contents-' + str(i)
      self._slides[contents_name].markdown += output[i]

  ##
  # Return the complete markdown for this slide set
  def getMarkdown(self):

    # Create a list of all the slide markdown
    output = []

    # Extract the slide content
    for name in self.activeSlides():
      output.append(self._slides[name].getMarkdown())

    # Join the list with slide breaks
    if self._warehouse.format == 'remark':
      output = '\n---\n'.join(output)
    elif self._warehouse.format == 'reveal':
      output = '\n\n'.join(output)

    # Append the links
    for link in self._links:
      output += link + '\n'
    return output
