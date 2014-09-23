# Load python packages
import re, sys
from collections import OrderedDict

# Load Moose packages
from FactorySystem import MooseObject
from src.slides import *

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
    params.addParam('slides', 'A list of slide ids to output, if blank all slides are output')
    params.addParam('contents', 'Include table of contents slide')
    params.addParam('contents_level', 1, 'The heading level to include in the contents')
    params.addParam('show_in_contents', True, 'Toggle if the slide set content appears in the table-of-contents')

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
    self._slides = OrderedDict()

    # Count the number of slides, used for assigning default name
    self._count = 0

    # Print a message
    print 'Created:', name


  ##
  # The method that creates/retrieves the markdown (virtual)
  def read(self):
    return ''


  ##
  # Initial setup creates the title and contents slide, if desired
  def setup(self):

    # Apply title slide
    if self.isParamValid('title'):
      self._createSlide('# ' + self.getParam('title') + '\n', show_in_contents=False, title=True,
                       name=self.name() + '-title')

    # Add table-of-contents slide
    if self.isParamValid('contents'):
      self._createSlide('# Contents\n', name = self.name() + '-contents', show_in_contents=False)


  ##
  # Creates slides from the raw_markdown input
  # @param raw_markdown The RemarkJS markdown to parse into slides
  def build(self, raw_markdown):

    # Do nothing if the raw_markdown is empty
    if not raw_markdown:
      return

    # Separate the individual slides
    raw_slides = re.split(r'\n---', raw_markdown)

    # Create/store the slides
    for raw in raw_slides:
      self._createSlide(raw)


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
    print '  creating slide:', name

    # Get the default input parameters from the slide being created
    params = self._factory.validParams(self._slide_type)

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
          print '    setting slide parameters from input file'
          self._parser.extractParams('', params, node)

    # Add the parent and markdown parameters
    params.addPrivateParam('_parent', self)
    params.addRequiredParam('markdown', raw, 'The raw markdown to parse for the current slide')

    # Over-ride parameters with optional key, value pairs
    for key, value in kwargs.iteritems():
      params[key] = value

    # Build and store the slide
    slide = self._factory.create(self._slide_type, name, params)
    self._slides[name] = slide
    self._count += 1

    # Call the parse method and populate the markdown
    slide._createImages(raw)
    slide.markdown = slide.parse(raw)
    slide._insertImages()


  ##
  # Build the table-of-contents
  def contents(self):

    # Do nothing if the 'contents' flag is not set in the input file
    if not self.isParamValid('contents'):
      return

    lvl = int(self.getParam('contents_level'))

    # Extract the contents
    contents = []
    for slide in self._slides.itervalues():
      contents += slide.contents()

    # Get the slide object that contains the contents slide
    slide = self._slides[self.name() + '-contents']

    # Build the table-of-contents entries
    output = ''
    for item in contents:
      if item[2] <= lvl:
        title = item[0]         # the heading content
        name = item[1]          # slide name
        indent = 25*(item[2]-1) # heading level indenting
        idx = str(item[3])      # slide index

        if slide.isParamValid('line-height'):
          height = slide.getParam('line-height')
        else:
          height = '15px'

        # Build a link to the slide, by name
        link = '<a href="#' + name + '">'

        # Create the contents entry
        output += '<p style="line-height:' + height + ';text-align:left;text-indent:' + str(indent) + 'px;">' + link + title + '</a>'
        output += '<span style="float:right;">' + link + idx + '</a>'
        output += '</span></p>\n'

    # Populate the markdown for the contents slide
    slide.markdown += output


  ##
  # Return the complete markdown for this slide set
  def getMarkdown(self):

    # Create a list of all the slide markdown
    output = []
    for value in self._slides.itervalues():
      output.append(value.getMarkdown())

    # Join the list with slide breaks
    return '\n---\n'.join(output)
