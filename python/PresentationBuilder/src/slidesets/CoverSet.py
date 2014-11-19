# Import the SlideSet base class
from src.slidesets import SlideSet

##
# A special set of slides for creating cover page and contents
class CoverSet(SlideSet):

  ##
  # Extract the valid parameters for this object
  @staticmethod
  def validParams():
    params = SlideSet.validParams()
    return params

  ##
  # Update the table-of-contents slide
  def contents(self):

    # Do nothing if the 'contents' flag is not set in the input file
    if not self.isParamValid('contents'):
      return

    # Loop through each object and append the markdown with title slides
    for obj in self._warehouse.objects:
      if obj != self:
        for slide in obj._slides.itervalues():
          if slide.getParam('title') and obj.getParam('show_in_contents'):
            link = '<a href="#' + slide.name() + '">'
            output = '<p style="text-align:left;">' + link + obj.getParam('title') + '</a>'
            output += '<span style="float:right;">' + link + str(slide.index) + '</a>'
            output += '</span></p>\n'
            self._slides[self.name() + '-contents'].markdown += output
