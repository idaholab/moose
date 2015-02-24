import sys, re
from FactorySystem import Warehouse


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

    # Read and build the slides
    print 'EXECUTE:'
    for obj in self.objects:
      name = obj.name()
      print '   READ:', name
      raw = obj.read()
      print '  BUILD:', name
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
    for obj in self.objects:
      for slide in obj.warehouse().activeObjects():
        slide.number = idx
        idx += 1 + len(re.findall('\n--', slide.markdown))

    # Call the contents object on each slide set
    for obj in self.objects:
      obj.contents()
