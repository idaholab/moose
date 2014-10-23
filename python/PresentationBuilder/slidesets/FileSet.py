from ..slidesets import SlideSet

##
# A basic class for reading Remark markdown files directly
class FileSet(SlideSet):

  ##
  # Constructor
  def __init__(self, name, params, **kwargs):
    SlideSet.__init__(self, name, params, slide_type='RemarkSlide')

  ##
  # Valid parameters for the FileSet object
  @staticmethod
  def validParams():
    params = SlideSet.validParams()
    params.addRequiredParam('file', 'The raw markdown file, in RemarkJS format, to build slide set from')
    return params

  ##
  # Read and return the raw markdown
  def read(self):
    filename = self.getParam('file')
    fid = open(filename, 'r')
    raw = fid.read()
    fid.close()
    return raw
