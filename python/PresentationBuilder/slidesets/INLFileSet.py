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
  # Initialize contents for INL slides
  # This creates injects the dark title slide and centers the title
  def initContents(self):
    FileSet.initContents(self) # base class initialization
    INLSetInterface.initContents(self) # format the contents for INL slides
