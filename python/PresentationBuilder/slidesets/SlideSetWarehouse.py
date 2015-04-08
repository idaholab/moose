import sys, re, sys
from FactorySystem import Warehouse
from utils import colorText


##
# A storage warehouse for SlideSet objects
class SlideSetWarehouse(Warehouse):

  ##
  # Constructor
  def __init__(self, **kwargs):
    Warehouse.__init__(self)

    self.format = kwargs.pop('format', 'remark')
    self.css = dict() # Storage for the css stylesheet


  ##
  # Returns the complete markdown for all SlideSets
  def markdown(self):

    # Extract all the slide set content
    output = []
    print colorText('\nRetrieving Markdown', 'CYAN')
    for obj in self.objects:
      md = obj.markdown()
      if not md:
        print 'Warning: The slide set, ' + obj.name() +', does not contain content.'
      else:
        output.append(md)

    if self.format == 'remark':
      return '\n\n---\n\n'.join(output)
    elif self.format == 'reveal':
      return '\n\n'.join(output)


  ##
  # Performs the slide creation steps
  def execute(self):

    # Number of slide set objects
    n = len(self.objects)

    # Read and build the slides
    for i in range(n):
      # Display the current object being executed
      obj = self.objects[i]
      name = obj.name()
      msg = ['Building Set:', name, '(' + str(i+1), 'of', str(n) + ')']
      print colorText(' '.join(msg), 'CYAN')

      # Read and build the content
      print '  Reading content...'
      raw = obj.read()
      print '  Building content...'
      obj.build(raw)

    # Build the table-of-contents
    self.__contents()


  ##
  # Builds the table of contents for each object (private)
  def __contents(self):

    # Initialize the table-of-contents slides
    for obj in self.objects:
      obj.initContents()

    # Initial slide index
    idx = 1
    title_slides = []

    # Loop through each object and slide and set the slide index
    print colorText('\nGenerating contents:', 'CYAN')
    for obj in self.objects:
      for slide in obj.warehouse().activeObjects():
        print '  ' + slide.name()
        slide.number = idx
        idx += 1 + len(re.findall('\n--', slide.markdown))

    # Call the contents object on each slide set
    for obj in self.objects:
      obj.contents()
