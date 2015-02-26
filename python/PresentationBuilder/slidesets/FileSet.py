from ..slidesets import RemarkSlideSet

##
# A basic class for reading Remark markdown files directly
class FileSet(RemarkSlideSet):

  ##
  # Constructor
  def __init__(self, name, params, **kwargs):
    RemarkSlideSet.__init__(self, name, params, **kwargs)

  ##
  # Valid parameters for the FileSet object
  @staticmethod
  def validParams():
    params = RemarkSlideSet.validParams()
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
