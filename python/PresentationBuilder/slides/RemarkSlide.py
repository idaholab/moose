# Python packages
import os, re, inspect
from FactorySystem import InputParameters, MooseObject
from ..images import *
##
# Base class for individual Remark markdown slide content
class RemarkSlide(MooseObject):

  # Regex for locating titles
  _title_re = r'(?<![^\s\n.])(#{1,})(.*?)\n'

  @staticmethod
  def validParams():
    params = MooseObject.validParams()
    params.addParam('class', 'The class property for RemarkJS CSS content')
    params.addParam('background-image', 'The background image file name')
    params.addParam('comments', 'Additional slide comments')
    params.addParam('show_in_contents', True, 'Toggle if the slide appears in the table-of-contents')
    params.addParam('prefix', 'Raw markdown to insert before slide content')
    params.addParam('suffix', 'Raw markdown to insert after slide content')
    params.addParam('auto_title', True, 'Enable/disable automatic addition of (cont.) for slides without a title.')

    params.addParamsToGroup('properties', ['class', 'background-image'])

    params.addPrivateParam('title', False)
    return params

  ##
  # Extracts the name of the object to be created
  @staticmethod
  def extractName(raw):
    # If the name is given in the slide markdown, use it
    match = re.search('(name:\s*(.*?)\s*\n)', raw)
    if match:
      return match.group(2)

    # Use the slide title
    match = re.search(RemarkSlide._title_re, raw)
    if match:
      return '-'.join(match.group(2).lower().split())

    return None

  ##
  # Constructor.
  # @param id The slide number
  # @param markdown The raw markdown for the slide
  # @param kwargs Optional Slide properties, see below
  #
  # RemarkJS utilizes a special property syntax to change the
  # look of individual slides, see the RemarkJS website for additional
  # details. These properties may be applied to the slide via
  # the optional key, value pair input in the constructor or by
  # modifying the 'parameters' member variable of the Slide class. For
  # example, adding "name='slide_name'" as an argument adds the following
  # to the markdown.
  #
  #  name: slide_name
  #
  # Additional Optional Parameters:
  #  comments = <str>
  #  The string supplied is implemented as a RemarkJS comment, which
  #  only appears in presentation mode. For example, using "comments='This
  #  is a comment'" results in the following being added to the markdown.
  #
  #   ???
  #   This is a comment
  #
  def __init__(self, name, params, **kwargs):
    MooseObject.__init__(self, name, params)

    # Set the Image type
    self._image_type = kwargs.pop('image_type', 'MarkdownImage')

    # Set the parent object
    self.parent = self.getParam('_parent') # the SlideSet parent object

    # Get a reference to the factory
    self._warehouse = self.parent.getParam('_warehouse')
    self._factory = self.parent.getParam('_factory')
    self._parser = self.parent.getParam('_parser')
    self._root = self.parent.getParam('_root')

    # Initialize member variables
    self.markdown = None                   # storage for parsed markdown (see SlideSet::createSlide)
    self.index = None                      # slide index, this is only populated and used for table of contents generation
    self.comments = []                     # comment storage for the current slide
    self._show_in_contents = self.getParam('show_in_contents')
    self._title = None
    self._previous = None
    self._raw_markdown = self.getParam('markdown')

    # Set the location of PresentationBuilder directory
    self._source_dir = os.path.abspath(os.path.join(os.path.split(inspect.getfile(self.__class__))[0], '..'))

    # Initialize the image information storage
    self._images = dict()
    self._createImages(self._raw_markdown)

    # Add comments supplied via the optional argument
    if self.isParamValid('comments'):
      self.comments.append(self.getParam('comments'))

    # Extract title
    match = re.search(self._title_re, self._raw_markdown)
    if match:
      self._title = (match.group(1) + ' ' + match.group(2)).replace('\r', '')


  ##
  # Build table of contents list
  def contents(self):

    # Return nothing if the contents flag is false
    if not self._show_in_contents:
      return []

    # Search the slide for headings
    pattern = re.compile(r'^\s*(#{1,})(.*?)\n')

    # Build a tuple containing the table-of-contents information for this slide
    contents = []
    for m in pattern.finditer(self.markdown):
      contents.append((m.group(2).strip(), self.name(), len(m.group(1)), self.index))
    return contents


  ##
  # Parse the markdown
  #
  # This method should overloaded to handle special syntax
  def parse(self, markdown):

    # Remove existing comments, they will be combined with other comments added via input file
    match = re.search(r'\?\?\?\s*\n(.*)', markdown, re.S)
    if match:
      self.comments.append(match.group(1))
      markdown = markdown.replace(match.group(0), '')

    # Search the raw markdown for an existing name
    match = re.search('(name:\s*(.*?)\s*\n)', markdown)
    if match:
      # Remove the name, it is added in the markdown() method when retrieving slide
      markdown = markdown.replace(match.group(1), '')

    # Search the raw markdown for an existing class
    for key in self._pars.groupKeys('properties'):
      match = re.search('(' + key + ':\s*(.*?)\s*\n)', markdown)
      if match:
        self._pars[key] = match.group(1)
        markdown = markdown.replace(match.group(1), '')


    # Return the parsed markdown
    return markdown

  ##
  # Creates the Image objects from the markdown
  def _createImages(self, markdown):

    # Get the class from the name of the slide
    image_class = globals()[self._image_type]

    # Register the class with the factory
    self._factory.register(image_class, self._image_type)

    # Apply the [./Slides] block
    images_node = None
    if self._root:
      images_node = self._root.getNode(self.parent.name()).getNode('Images')

    # Do nothing if the ./Images node does not exist
    if not images_node:
      return

    # Get the common parameters from the ./Images block
    parent_params = self._factory.validParams(self._image_type)
    self._parser.extractParams('', parent_params, images_node)

    # Build the image objects
    match_list = image_class.match(markdown)
    for match in match_list:
      for m in match:

        # Get the default parameters from the image being created
        params = self._factory.validParams(self._image_type)

        # Apply the common parameters
        params.applyParams(parent_params)

        # Add the parent parameter
        params.addPrivateParam('_parent', self)
        params.addPrivateParam('_match', m)

        # Extract the object name
        name = image_class.extractName(m)

        # Do nothing if the image was already created
        # I am not sure why the images are matched more than once, but they are...
        if name in self._images:
          continue

        # Indicate that the image is being created
        print ' '*6 + 'IMAGE:', name

        # Apply the [./Slides] block parameters
        if images_node:
          node = images_node.getNode(name)
          if node:

            # Apply the parameters from the node
            self._parser.extractParams('', params, node)

            # Indicate that the parameters are being set for the image
            print ' '*8 + 'Appling image settings from input file'

        # Create and store the Image object
        img = self._factory.create(self._image_type, name, params)
        self._images[name] = img


  ##
  # Extracts the markdown image and inserts the html image
  # @param tag The image id
  # @param params The image settings from the input file
  def _insertImages(self):
    for tag, image in self._images.iteritems():
      self.markdown = re.sub(image.sub(), image.html(), self.markdown)


  ##
  # Compute the number of slides
  #
  # Remark slides are indexed and include animation slides in the index, this
  # returns the total number of "slides" contained in the current object. This is
  # needed for generating the table-of-contents
  def count(self):
    return 1 + len(re.findall('\n--', self.markdown))


  ##
  # Return the RemarkJS ready markdown for this slide
  def getMarkdown(self):

    # The markdown to be output
    output = ''

    # Print the name
    output += 'name: ' + self.name() + '\n'

    # Insert the Remark slide properties
    for key in self._pars.groupKeys('properties'):

      # Print the slide properties
      if self.isParamValid(key):
        value = self.getParam(key)

        if key == 'background-image':
          # Assume the image is in the backgrounds directory if it doesn't exist
          if not os.path.exists(value):
            value = os.path.join(self._source_dir, 'backgrounds', value)

          # Error if the file does not exist
          if not os.path.exists(value):
            print 'ERROR: Background file "' + value + '" does not exist'

          output += key + ': url(' + value + ')\n'

        else:
          output += key + ': ' + value + '\n'

    # Insert prefix markdown
    if self.isParamValid('prefix'):
      output += self.getParam('prefix')

    # Insert continued title
    if self._title == None and self.getParam('auto_title'):
      idx = self.parent._slide_order.index(self.name())
      if idx > 0:
        previous_name = self.parent._slide_order[idx-1]
        previous = self.parent._slides[previous_name]
      else:
        previous = None

      if previous and previous._title != None:
        self._title = previous._title
        output += self._title + ' (cont.)'

    # Inject the the raw markdown
    output += self.markdown

    # Insert suffix markdown
    if self.isParamValid('suffix'):
      output += self.getParam('suffix')

    # Add the Remark comments
    if self.comments:
      output += '\n???\n'
      for comment in self.comments:
        output += comment + '\n'

    # Return the complete markdown
    return output
