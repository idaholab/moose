# Python packages
import os, re, inspect, string, urllib2, sys
from FactorySystem import InputParameters, MooseObject
from utils import colorText
from ..images import *
from ..tools import *

##
# Base class for individual Remark markdown slide content
class RemarkSlide(MooseObject):

  @staticmethod
  def validParams():
    params = MooseObject.validParams()
    params.addParam('class', 'The class property for RemarkJS CSS content')
    params.addParam('name', 'The name of the slide')
    params.addParam('background-image', 'The background image file name')
    params.addParam('comments', 'Additional slide comments')
    params.addParam('show_in_contents', True, 'Toggle if the slide appears in the table-of-contents')
    params.addParam('prefix', 'Raw markdown to insert before slide content')
    params.addParam('suffix', 'Raw markdown to insert after slide content')
    params.addParam('auto_title', True, 'Enable/disable automatic addition of (cont.) for slides without a title.')
    params.addParam('slides', 'A list of slides to include')
    params.addParam('auto_insert_github_code', True, 'When true links to GitHub code is automatically inserted')
    params.addParam('insert_github_code_link', 'top', 'When auto linking code place the link at with the code (top | bottom | none)')

    # Remark properties that should remain in the raw markdown
    params.addParamsToGroup('properties', ['name', 'class', 'background-image'])

    params.addPrivateParam('title', False)
    return params


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
  def __init__(self, params, **kwargs):
    MooseObject.__init__(self, params)

    # Set the Image type
    self.__image_type = kwargs.pop('image_type', 'MarkdownImage')

    # Set the parent object, which is the RemarkSlideSet parent object
    self.parent = self.getParam('_parent')

    # The slide number, this is set by the SlideSetWarehouse::__contents method
    self.number = 0

    # Get a reference to the factory, parser, and the root node of the input file
    self.__factory = self.parent.getParam('_factory')
    self.__parser = self.parent.getParam('_parser')
    self.__root = self.parent.getParam('_root')

    # Storage for raw markdown, this is populated by the parent RemarkSlideSet
    self.markdown = None

    # Storage for the comments
    self.comments = []

    # Storage for the slide title
    self.__title = None

    # Initialize the image information storage
    self.__images = dict()

    # Source directory for the PresentationBuilder code
    self.__source_dir = os.path.abspath(os.path.join(os.path.split(inspect.getfile(self.__class__))[0], '..'))

    # Add comments supplied via the optional argument
    if self.isParamValid('comments'):
      self.comments.append(self.getParam('comments'))


  ##
  # Return the tuple title of slide (public)
  #
  # The first items is the heading level (i.e., '#' or '##', etc...)
  # The second item is the actual text of the title
  #
  # This will not return anything useful until after the parse method is called
  def title(self):
    return self.__title


  ##
  # Do initial parsing to extract the name of the slide
  def parseName(self, markdown):

    # Remove un-wanted line endings
    markdown = markdown.replace('\r', '')

    # Search the raw markdown for an existing settings
    for match in re.finditer(r'(\w+)\s*:\s*(.*?)\n', markdown):
      key = match.group(1)
      if key in self.parameters():
        value = match.group(2)
        if value.lower() == 'true' or value == '1':
          value = True
        if value.lower() == 'false' or value == '0':
          value = False
        self.parameters()[key] = value
        markdown = markdown.replace(match.group(0), '')

    # Locate the title of the slide
    match = re.search(r'^\s*(#+)\s+(.*)\n', markdown, flags = re.MULTILINE)
    if match:
      self.__title = (match.group(1), match.group(2))

    # Set the object name using the title, if the name parameter does not exist
    if not self.isParamValid('name') and (self.__title is not None):
      self.parameters()['name'] = '-'.join(self.__title[1].lower().split()) + '-0'

    # Set the object name using the previous slide name , if the name parameter does not exist
    elif not self.isParamValid('name') and self.parent.warehouse().objects:
      previous = self.parent.warehouse().objects[-1]
      if previous and previous.name():

        # Set the name of the slide with the correct number prefix based on the previous slide
        name = previous.name() + '-1'
        match = re.search('(.*?)(\d+)$', previous.name())
        if match:
          name = match.group(1) + str(int(match.group(2))+1)
        self.parameters()['name'] = name

    # If the above attempts at naming fail, name the object based on the containing slide set
    elif not self.isParamValid('name'):
      name = self.parent.name() + '-' + str(self.parent.warehouse().numObjects())
      self.parameters()['name'] = name

    return markdown


  ##
  # Parse the markdown
  #
  # This method should overloaded to handle special syntax
  def parse(self, markdown):

    # Insert prefix markdown
    if self.isParamValid('prefix'):
      markdown = self.getParam('prefix') + '\n' + markdown

    # Print a message of the slide that is being created
    print ' '*4, 'SLIDE:', self.name()
    return markdown


  ##
  # Parse the markdown
  #
  # This method should overloaded to handle special syntax
  def parse(self, markdown):

    # Insert prefix markdown
    if self.isParamValid('prefix'):
      markdown = self.getParam('prefix') + '\n' + markdown

    # Apply continued title
    if (self.__title is None) and self.getParam('auto_title'):
      if self.parent.warehouse().objects:
        previous = self.parent.warehouse().objects[-1]
        if previous and previous.title():
          if previous.title()[1].endswith('(cont.)'):
            self.__title = previous.title()
          else:
            self.__title = (previous.title()[0], previous.title()[1] + ' (cont.)')

          # Insert the title into the markdown
          self.parameters()['show_in_contents'] = False
          if self.__title:
            markdown = self.__title[0] + ' ' + self.__title[1] + '\n' + markdown

    # Insert suffix markdown
    if self.isParamValid('suffix'):
      markdown += self.getParam('suffix')

    # Remove existing comments, they will be combined with other comments added via input file
    match = re.search(r'\?\?\?\s*\n(.*)', markdown, re.S)
    if match:
      self.comments.append(match.group(1))
      markdown = markdown.replace(match.group(0), '')

    # Adjust the background-image parameter
    if self.isParamValid('background-image'):
      value = self.getParam('background-image')

      # Assume the image is in the idaholab/presentations/moose_workshop directory if it doesn't exist
      if not os.path.exists(value):
        value = os.path.join('https://mooseframework.org/static/media/uploads/images/backgrounds', value)

      # Update the parameter
      self.parameters()['background-image'] = 'url(' + value + ')'

    # Re-insert the Remark properties, they are needed for Remark to create links
    for key in self.parameters().groupKeys('properties'):
      if self.isParamValid(key):
          markdown = key + ':' + self.parameters()[key] + '\n' + markdown

    # Adjust code starting with *
    # Remark automatically highlights code lines starting with *, this is disabled
    # by removing the background in the css and adding an extract *, I hope Remark
    # will get updated so this is not necessary.
    regex = r'```.*?```'
    markdown = re.sub(regex, self.__subLineHighlight, markdown, flags = re.DOTALL | re.MULTILINE)

    # Search for github code urls
    if self.getParam('auto_insert_github_code'):

      # Parse code with function extraction
      regex = '(\[.*?\]\((https://github.com/(.*?/)blob/(.*?))#(.*?)\))'
      markdown = re.sub(regex, self.__subGitHubCode, markdown)

      # Parse complete code
      regex = '(\[.*?\]\((https://github.com/(.*?/)blob/(.*?))\))'
      markdown = re.sub(regex, self.__subGitHubCode, markdown)

    # Error if the "name" is not set at this point
    if not self.isParamValid('name'):
      self.parameters()['name']
      print '\nERROR: Unable to parse the name from the following markdown.'
      print '--------------------------------------------------'
      print markdown
      print '--------------------------------------------------'
      raise Exception('The name of the slide was not able to be set')

    # Return the markdown
    return markdown


  ##
  # Substitution method for fixing auto line highlighting
  def __subLineHighlight(self, match):
    cpp_section = match.group(0)
    # Append asterisks at the front of lines beginning with asterisks
    return re.sub('^(\*)', # Capture literal asterisk starting a line \
                  r'*\1',  # Replace with a new asterisk followed by the captured asterisk \
                  cpp_section, flags = re.MULTILINE | re.VERBOSE)


  ##
  # Substitution method for github code
  def __subGitHubCode(self, match):

    # Download the code
    url = 'https://raw.githubusercontent.com/' + match.group(3) + match.group(4)
    response = urllib2.urlopen(url)
    code = response.read()

    # Determine language
    language = ''
    _, ext = os.path.splitext(url)
    if ext in ['.C', '.h']:
      language = 'cpp'
    elif ext == '.py':
      language = 'python'
    elif ext == '.i':
      language = 'text'
    # Do nothing if the language is not detected
    else:
      return match.group(0)

    # Remove header
    strt = code.find('/********')
    stop = code.rfind('*******/\n')
    code = code.replace(code[strt:stop+9], '')

    # Strip code (i.e, remove functions, prototypes and input file blocks)
    if len(match.groups()) == 5:
      strip = match.group(5)
      if strip in code:
        code = stripCode(code, ext, strip)

    # Initialize the output string
    block = ''

    # Insert link at top
    link = self.getParam('insert_github_code_link')
    if link == 'top':
      block += match.group(0) + '\n'

    # Insert code
    block += '\n```' + language + '\n'
    block += code
    block += '```\n'

    # Insert link at bottom
    if link == 'bottom':
      block += match.group(0)

    return block

  ##
  # Creates the Image objects from the markdown (public)
  def parseImages(self, markdown):

    # Apply the [./Slides] block
    images_node = None
    if self.__root:
      images_node = self.__root.getNode(self.parent.name()).getNode('Images')

    # Get the common parameters from the ./Images block
    parent_params = self.__factory.validParams(self.__image_type)
    if images_node:
      self.__parser.extractParams('', parent_params, images_node)

    # Build the image objects
    image_class = globals()[self.__image_type]
    for match in image_class.match(markdown):

      # Get the default parameters from the image being created
      params = self.__factory.validParams(self.__image_type)

      # Apply settings coming from the image wiki line
      # i.e. ![caption](image.png align:right width:500px)
      for pair in match['settings'].split():
        k,v = pair.strip().split(':')
        if k in params:
          params[k] = v

      # Set the required parameters
      if not params.isValid('name'):
        params['name'] = match['name']

      if not params.isValid('caption'):
        params['caption'] = match['caption']

      if not params.isValid('url'):
        params['url'] = match['url']

      # The image object name
      name = params['name']

      # Apply the common parameters
      params.applyParams(parent_params)

      # Add the parent parameter
      params.addPrivateParam('_parent', self)

      # Indicate that the image is being created
      print ' '*6 + 'IMAGE:', name

      # Apply the [./Slides] block parameters
      if images_node:
        node = images_node.getNode(name)
        if node:

          # Apply the parameters from the node
          self.__parser.extractParams('', params, node)

          # Indicate that the parameters are being set for the image
          print ' '*8 + 'Appling image settings from input file'

      # Create and store the image object
      image = self.__factory.create(self.__image_type, name, params)
      self.__images[name] = image

      # Replace the image with html
      markdown = markdown.replace(match['markdown'], '\n' + image.html())

    return markdown
