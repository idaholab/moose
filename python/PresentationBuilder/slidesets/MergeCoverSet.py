# Import the SlideSet base class
import math
from ..slidesets import SlideSet

##
# A special set of slides for creating cover page and contents
class MergeCoverSet(SlideSet):

  ##
  # Extract the valid parameters for this object
  @staticmethod
  def validParams():
    params = SlideSet.validParams()
    params.addParam('slide_sets', 'A vector of slideset names to combine into a single contents')
    return params

  def __init__(self, name, params, **kwargs):
    SlideSet.__init__(self, name, params)

    self._merge_list = []
    if self.isParamValid('slide_sets'):
      self._merge_list = self.getParam('slide_sets')

  ##
  # Search through all the slides in the specified slide sets for table of contents content
  def _extractContents(self):

    # Count the number of contents entries
    contents = []
    for obj in self._warehouse.objects:
      if obj != self and (len(self._merge_list) == 0 or obj.name() in self._merge_list):
        for name in obj.activeSlides():
          contents += obj._slides[name].contents()

    return contents
