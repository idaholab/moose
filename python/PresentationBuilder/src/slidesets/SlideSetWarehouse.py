import sys
from FactorySystem import Warehouse


##
# A storage warehouse for SlideSet objects
class SlideSetWarehouse(Warehouse):

  ##
  # Returns the complete markdown for all SlideSets
  def markdown(self):
    output = [obj.getMarkdown() for obj in self.objects]
    return '\n\n---\n\n'.join(output)

  ##
  # Performs the slide creation steps
  def execute(self):
    for obj in self.objects:
      name = obj.name()
      print 'Setup:', name
      obj.setup()

      print 'Read:', name
      raw = obj.read()

      print 'Build:', name
      obj.build(raw)
