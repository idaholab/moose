import sys, os, inspect

from FactorySystem import *
from src.slidesets import *
from src.slides import *

##
# Class for generating complete slide shows from markdown syntax
class PresentationBuilder(object):

  ##
  # Constructor
  # @param input_file The name of the input file to read
  def __init__(self, input_file):

    # Create the Factory and Warehouse
    self.factory = Factory()
    self.warehouse = SlideSetWarehouse()

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
    err = self.parser.parse(input_file)
    if err:
      sys.exit()

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

    fid = open(os.path.join(folder, '..', '..', 'templates', name + '.html'), 'r')
    data = fid.read()
    fid.close()

    # Inject CSS into the html header material
    if name == 'top':
      data = self._injectCSS(data)

    # Return the html
    return data


  ##
  # Inserts additional CSS commands into the html output
  # @param data The raw html to inject the css code into
  # @see _template
  def _injectCSS(self, data):

    # Look for the CSS block in the input file, do nothing if it is not found
    node = self.parser.root.getNode('CSS')
    if not node:
      return data

    # Locate the point at which the custom css should be injected
    idx = data.find('</style>')

    # Write the intial portion of the html
    output = data[0:idx-1] + '\n'

    # Write the custom CSS code
    for name, block in node.children.iteritems():
      output += ' '*6 + '.' + name + ' {\n'
      for key, value in block.params.iteritems():
        output += ' '*8 + key + ': ' + str(value) + ';\n'
      output += ' '*6 + '}\n'

    # Write the remaining html and return it
    output += data[idx:]
    return output


  ##
  # Builds the table of contents for each object
  def _contents(self):

    # Initial slide index
    idx = 1
    title_slides = []

    # Loop through each object and slide and set the slide index
    for obj in self.warehouse.objects:
      for slide in obj._slides.itervalues():
        slide.index = idx
        idx += slide.count()

    # Call the contents object on each slide set
    for obj in self.warehouse.objects:
      obj.contents()
