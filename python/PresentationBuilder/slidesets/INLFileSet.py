from ..slidesets import FileSet, INLSetInterface

##
# A basic class for reading Remark markdown files directly
class INLFileSet(FileSet, INLSetInterface):

  ##
  # Valid parameters for the FileSet object
  @staticmethod
  def validParams():
    params = FileSet.validParams()
    params += INLSetInterface.validParams()
    return params

  ##
  # Performs INL slide parsing
  def _parseSlide(self, slide, raw):
    INLSetInterface.applySlideSettings(slide)
    FileSet._parseSlide(self, slide, raw)
