# Import the SlideSet base class
import math
from ..slidesets import CoverSet, INLSetInterface

##
# A special set of slides for creating cover page and contents in INL presentation format
class INLCoverSet(CoverSet, INLSetInterface):

  ##
  # Valid parameters for the WikiSet class
  @staticmethod
  def validParams():
    params = CoverSet.validParams()
    params += INLSetInterface.validParams()
    return params

  ##
  # Initialize contents for INL slides
  # This creates injects the dark title slide and centers the title
  def initContents(self):
    CoverSet.initContents(self) # base class initialization
    INLSetInterface.initContents(self) # format the contents for INL slides
