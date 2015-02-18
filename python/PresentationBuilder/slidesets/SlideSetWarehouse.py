import sys
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
      markdown = obj.getMarkdown()
      if not markdown:
        print 'Warning: The slide set, ' + obj.name() +', does not contain content.'
      else:
        output.append(markdown)

    if self.format == 'remark':
      return '\n\n---\n\n'.join(output)
    elif self.format == 'reveal':
      return '\n\n'.join(output)

  ##
  # Performs the slide creation steps
  def execute(self):

    print 'EXECUTE:'

    for obj in self.objects:
      name = obj.name()
      print '  SETUP:', name
      obj.setup()

      print '   READ:', name
      raw = obj.read()

      print '  BUILD:', name
      obj.build(raw)
