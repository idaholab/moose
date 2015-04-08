# Import the SlideSet base class
import math
from ..slidesets import MergeCoverSet, INLSetInterface

##
# A special set of slides for creating cover page and contents
class INLMergeCoverSet(MergeCoverSet, INLSetInterface):

  ##
  # Valid parameters for the WikiSet class
  @staticmethod
  def validParams():
    params = MergeCoverSet.validParams()
    params += INLSetInterface.validParams()
    return params

  ##
  # Performs INL slide parsing
  def _parseSlide(self, slide, raw):
    INLSetInterface.applySlideSettings(slide)
    MergeCoverSet._parseSlide(self, slide, raw)
