# Load python packages
import re, sys, math
from collections import OrderedDict

# Load Moose packages
from FactorySystem import MooseObject
from ..slides import RemarkSlide, SlideWarehouse

##
# Base class for markdown slide generation
class RemarkSlideSet(MooseObject):

  ##
  # Defines the available properties for the SlideSet base class
  @staticmethod
  def validParams():
    params = MooseObject.validParams()
    params.addRequiredParam('type', 'The type of slide set to create')
    params.addParam('title', 'The title of the slide set, if this exists a title slide will be injected')
    params.addParam('active', [], 'A list of ordered slide names to output, if blank all slides are output')
    params.addParam('inactive', [], 'A list of slide names to exclude from output')
    params.addParam('contents', False, 'Include table of contents slide')
    params.addParam('contents_title', 'The table-of-contents heading for this slide set')
    params.addParam('contents_level', 1, 'The heading level to include in the contents')
    params.addParam('contents_items_per_slide', 12, 'The number of contents items to include on a page')
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

    # Set the Slide type
    self.__slide_type = kwargs.pop('slide_type', 'RemarkSlide')

    # Get a reference to the items needed to create objects
    self.__factory = self.getParam('_factory')
    self.__parser = self.getParam('_parser')
    self.__root = self.getParam('_root')

    # Create a storage object for the slides created by this set
    self.__slide_warehouse = SlideWarehouse(set_name = name, \
                                            active = self.getParam('active'), \
                                            inactive = self.getParam('inactive'))

    # Print a message
    print '  ', name


  ##
  # The method that creates/retrieves the markdown (virtual)
  def read(self):
    return ''


  ##
  # Returns a reference to the SlideWarehouse object
  def warehouse(self):
    return self.__slide_warehouse


  ##
  # Creates the individual RemarkSlide objects
  # @param raw The raw markdown, obtained from read() method, to seperate into slides
  def build(self, markdown):

    # Separate the slide content
    raw_slides = re.split(r'\n---', markdown)

    # Build the individual slide objects
    for raw in raw_slides:
      if raw:
        slide = self.__createSlide(raw)
        self.warehouse().addObject(slide)

    # Create the title slide
    if self.isParamValid('title'):
      name = self.name() + '-title'
      raw = '# ' + self.getParam('title') + '\n'
      options = {'show_in_contents':False, 'title':True, 'name':name, 'class':'center,middle'}
      slide = self.__createSlide(raw, **options)
      self.warehouse().insertObject(0, slide)


  ##
  # Return the complete markdown for this slide set
  def markdown(self):

    # Create a list of all the slide markdown
    output = []

    # Extract the slide content
    for slide in self.warehouse().activeObjects():
      output.append(slide.markdown)

    # Join the list with slide breaks
    output = '\n---\n'.join(output)

#    # Append the links
#    for link in self._links:
#      output += link + '\n'
    return output


  ##
  # Create the a slide from raw markdown (private)
  # @param raw The raw markdown to build the slide from
  # @param kwargs Optional key, value pairs
  #
  def __createSlide(self, raw, **kwargs):

    # Get the default input parameters from the slide being created
    params = self.__factory.validParams('RemarkSlide')
    params.applyParams(self.parameters())

    # Apply the common properties from this class
    #for key in params.groupKeys('properties'):
    #  if self.isParamValid(key):
    #    params[key] = self.getParam(key)

    # Add the parent and markdown parameters
    params.addPrivateParam('_parent', self)

    # Over-ride parameters with optional key, value pairs
    for key, value in kwargs.iteritems():
      params[key] = value

    # Build the slide object
    slide = self.__factory.create(self.__slide_type, params)

    # Determine and set the slide name
    raw = slide.parseName(raw)

    # Apply the [./Slides] block
    if self.__root:
      node = self.__root.getNode(self.name()).getNode('Slides')
      if node:
        node = node.getNode(slide.name())
        if node:
          print ' '*6 + 'Apply settings from input file'
          self.__parser.extractParams('', slide.parameters(), node)

    # Parse the raw markdown and store it in the slide
    self._parseSlide(slide, raw)
    return slide


  ##
  # Method that calls the various parse methods for the slide content (protected)
  # This also applies settings from the input file, this method exists to
  # allow parent classes to modify slide settings
  # @see INLDjangoWikiSet, INLCoverSet, INLMergeSet
  def _parseSlide(self, slide, raw):

    # Parse the content into Remark format and store the content in the slide
    raw = slide.parse(raw)
    raw = slide.parseImages(raw)
    slide.markdown = raw


  ##
  # A helper that extracts the contents entries from each of the active slides (protected)
  def _extractContents(self):

    contents = []

    # Loop through all active slides
    for slide in self.warehouse().activeObjects():

      # Do nothing if the contents for the slides are disabled
      if not slide.getParam('show_in_contents'):
        continue

      # Build a tuple containing the table-of-contents information for this slide
      pattern = re.compile(r'^\s*(#+)\s+(.*)', re.MULTILINE)
      for m in pattern.finditer(slide.markdown):
        contents.append((m.group(2).strip(), slide.name(), len(m.group(1)), slide.number))

    # Separate contents into chunks based on the allowable size
    n = int(self.getParam('contents_items_per_slide'))
    output = [contents[i:i+n] for i in range(0, len(contents),n)]
    return output

  ##
  # A helper method that creates the empty contents slides (protected)
  # @param number The number of contents entries
  def _createContentsSlides(self, n):

    # Determine the table of contents header
    if self.isParamValid('contents_title'):
      contents_title = '# ' + self.getParam('contents_title') + '\n'
    elif self.isParamValid('title'):
      contents_title = '# ' + self.getParam('title') + ' Contents\n'
    else:
      contents_title = '# Contents\n'

    # Locate the slide insert location
    if self.warehouse().hasObject(self.name() + '-title'):
      idx = 1
    else:
      idx = 0

    # Add the content(s) slides
    for i in range(n):
      name = '-'.join([self.name(), 'contents', str(i)])
      options = {'name' : name, 'show_in_contents' : False}
      if i == 0:
        slide = self.__createSlide(contents_title, **options)
      else:
        slide = self.__createSlide('', **options)
      self.warehouse().insertObject(idx, slide)
      idx += 1


  ##
  # Initialize contents (public)
  # This creates and inserts the correct number of contents slides
  # @see SlideSetWarehouse::__contents
  def initContents(self):

    # Do nothing if the 'contents' flag is not set in the input file
    if not self.getParam('contents'):
      return

    # Extract the contents entries
    contents = self._extractContents()

    # Create the contents slides
    self._createContentsSlides(len(contents))


  ##
  # Inserts the table-of-contents html into the already existing contents slides (public)
  # @see SlideSetWarehouse::__contents
  def contents(self):

    # Do nothing if the 'contents' flag is not set in the input file
    if not self.getParam('contents'):
      return

    # Extract the contents entries
    contents = self._extractContents()

    # Build the table-of-contents entries
    max_per_slide = int(self.getParam('contents_items_per_slide'))
    lvl = int(self.getParam('contents_level'))
    for i in range(len(contents)):
      output = ''
      for item in contents[i]:
        if item[2] <= lvl:
          title = item[0]         # the heading content
          name = item[1]          # slide name
          indent = 25*(item[2]-1) # heading level indenting
          idx = str(item[3])      # slide index
          height = '12px'

          # Build a link to the slide, by name
          link = '<a href="#' + name + '">'

          # Create the contents entry
          output += '<p style="line-height:' + height + ';text-align:left;text-indent:' + str(indent) + 'px;">' + link + title + '</a>'
          output += '<span style="float:right;">' + link + idx + '</a>'
          output += '</span></p>\n'


      # Write the contents to the slide
      name = '-'.join([self.name(), 'contents', str(i)])
      self.warehouse().getObject(name).markdown += output
