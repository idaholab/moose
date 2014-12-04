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
  # Initialize contents for INL slides
  # This creates injects the dark title slide and centers the title
  def initContents(self):
    MergeCoverSet.initContents(self) # base class initialization
    INLSetInterface.initContents(self)
