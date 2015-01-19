import sys, os, inspect, re

from collections import OrderedDict

from FactorySystem import *
from ..slidesets import *
from ..slides import *

##
# Class for generating complete slide shows from markdown syntax
#
# Available properties in top-level "[presentation]" block
#   style = 'inl'
#   code = 'dark' (see https://highlightjs.org/ for available options)
#
class PresentationBuilder(object):

  ##
  # Constructor
  # @param input_file The name of the input file to read
  def __init__(self, input_file, **kwargs):

    # Determine the format
    self._format = kwargs.pop('format', 'remark')

    self._reveal_dir = None
    if self._format == 'reveal':
      self._reveal_dir = os.getenv('REVEAL_JS_DIR', os.path.join(os.getenv('HOME'), 'projects', 'reveal.js'))

      if not os.path.exists(self._reveal_dir):
        print 'ERROR: Attempted to output in Reveal.js format, but Reveal.js directory was not found, set the REAVEL_JS_DIR enviornmental variable or clone the repository into your ~/projects directory.'
        sys.exit()


    # Create the Factory and Warehouse
    self.factory = Factory()
    self.warehouse = SlideSetWarehouse(format=self._format)

    # Set the location of PresentationBuilder directory
    self._source_dir = os.path.abspath(os.path.join(os.path.split(inspect.getfile(self.__class__))[0], '..'))

    # Extract input/output file names
    f, ext = os.path.splitext(input_file)
    self._input_file = input_file
    self._output_file = f + '.html'

    # Register the objects to be created
    self.factory.loadPlugins('src', 'slidesets', SlideSet)
    self.factory.loadPlugins('src', 'slides', RemarkSlide)

    # Build the Parser object
    self.parser = Parser(self.factory, self.warehouse)

    # Create SlideSet objects via the Parser object by parsing the input file
    print 'PARSER:'
    err = self.parser.parse(input_file)
    if err:
      sys.exit()
    print ''

    # Store the top-level 'presentation' level parameters
    self._params = self.parser.root.children["presentation"].params

    # Extract CSS style
    self._style()

    # Build the slides
    self.warehouse.execute()

    # Build the table-of-contents
    self._contents()


  ##
  # Write the html file containing the presentation (public)
  def write(self):
    fid = open(self._output_file, 'w')
    fid.write(self._template('top'))
    fid.write(self.warehouse.markdown())
    fid.write(self._template('bottom'))


  ##
  # Extracts the html templates that build the Remark slideshow (private)
  # @param name The name of the html file to extract
  def _template(self, name):

    # Open and read the template html
    folder, filename = os.path.split(inspect.getfile(self.__class__))

    fid = open(os.path.join(folder, '..', 'templates', self._format + '_' + name + '.html'), 'r')
    data = fid.read()
    fid.close()

    # Update the paths for Reveal.js contents
    if self._format == 'reveal':
      data = data.replace('../..', self._reveal_dir)

    # Inject CSS into the html header material
    if name == 'top':

      if self._format == 'remark':
        data = self._injectCSS(data)

      if 'title' in self._params:
        data = re.sub(r'\<title\>.*?\</title\>', '<title>' + self._params['title'] + '</title>', data)

    # Insert the code format
    if name == 'bottom' and self._format == 'remark' and 'code' in self._params:
      print 'hello'
      data = re.sub(r"(highlightStyle:.*?\,)", "highlightStyle: '" + self._params['code'] + "',", data)

    # Return the html
    return data

  ##
  # Get the CSS style information
  def _style(self):

    # If style if defined, extract it
    if 'style' in self._params:
      style = self._params['style']

      # Check if the style is a readable file, if not assume it is a name
      # of a file in the styles directory
      if not os.path.exists(style):
        style = os.path.join(self._source_dir, 'styles', style + '.css')

      # Display a message if the style file doesn't exists and do nothinbg
      if not os.path.exists(style):
        print 'ERROR: Style sheet file "' + style + '" does not exists'

      # Read the CSS and store the css in a dict() within the warehouse (needed for Reveal format)
      else:
        fid = open(style)
        css = fid.read()
        fid.close()
        css = re.sub(r'/\*(.*)\*/', '', css)
        match = re.findall(r'(.*?)\{(.*?)\}', css, re.S)
        style = OrderedDict()
        for m in match:
          style[m[0].strip()] = m[1].replace('\n', '').strip()

        self.warehouse.css = style

  ##
  # Inserts additional CSS commands into the html output
  # @param data The raw html to inject the css code into
  # @see _template
  def _injectCSS(self, data):

    # Locate the point at which the custom css should be injected
    idx = data.find('</style>')

    # Write the intial portion of the html
    output = data[0:idx-1] + '\n'

    # Inject the style
    for key, value in self.warehouse.css.iteritems():
      output += ' '*6 +  key + ' { ' + value + ' }\n\n'

    # Look for the CSS block in the input file, do nothing if it is not found
    node = self.parser.root.getNode('CSS')
    if node:

      # Write the custom CSS code
      for name, block in node.children.iteritems():
        output += ' '*6 + '.' + name + ' {\n'
        for key, value in block.params.iteritems():
          output += ' '*8 + key + ': ' + str(value) + ';\n'
        output += ' '*6 + '}\n'

    # Write the remaining html and return it
    output += ' '*4
    output += data[idx:]
    return output


  ##
  # Builds the table of contents for each object
  def _contents(self):

    # Initialize the table-of-contents slides
    for obj in self.warehouse.objects:
      obj.initContents()

    # Initial slide index
    idx = 1
    title_slides = []

    # Loop through each object and slide and set the slide index
    for obj in self.warehouse.objects:
      for name in obj.activeSlides():
        slide = obj._slides[name]
        slide.index = idx
        idx += slide.count()

    # Call the contents object on each slide set
    for obj in self.warehouse.objects:
      obj.contents()
