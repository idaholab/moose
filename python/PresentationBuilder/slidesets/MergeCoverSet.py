# Import the SlideSet base class
import math
from ..slidesets import RemarkSlideSet

##
# A special set of slides for creating cover page and contents
class MergeCoverSet(RemarkSlideSet):

  ##
  # Extract the valid parameters for this object
  @staticmethod
  def validParams():
    params = RemarkSlideSet.validParams()
    params.addRequiredParam('slide_sets', 'A vector of slideset names to combine into a single contents')
    return params

  def __init__(self, name, params, **kwargs):
    RemarkSlideSet.__init__(self, name, params)

    # The SlideSetWarehoue
    self.__warehouse = self.getParam('_warehouse')

    # Build a list of sets to merge
    self.__merge_list = self.getParam('slide_sets')

  ##
  # Search through all the slides in the specified slide sets for table of contents content
  def _extractContents(self):
    # Count the number of contents entries
    contents = []
    for obj in self.__warehouse.objects:
      if obj != self and (len(self.__merge_list) == 0 or obj.name() in self.__merge_list):
        pages = obj._extractContents()
        for page in pages:
          contents += page

    n = int(self.getParam('contents_items_per_slide'))
    output = [contents[i:i+n] for i in range(0, len(contents),n)]
    return output
