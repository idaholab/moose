# Import the SlideSet base class
import math
from ..slidesets import SlideSet

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
  #
  def initContents(self):

    # Count the number of contents entries
    cnt = 0
    for obj in self._warehouse.objects:
      if obj != self:
        for key in obj._slide_order:
          slide = obj._slides[key]
          if slide.getParam('title') and obj.getParam('show_in_contents'):
            cnt += 1

    # Determine the number of contents slides needed
    max_per_slide = float(self.getParam('contents_items_per_slide'))
    n = int(math.ceil(cnt / max_per_slide))

    # Determine the table of contents header
    if self.isParamValid('contents_title'):
      contents_title = '# ' + self.getParam('contents_title') + '\n'
    elif self.isParamValid('title'):
      contents_title = '# ' + self.getParam('title') + ' Contents\n'
    else:
      contents_title = '# Contents\n'

    # Locate the slide insert location
    if self.name() + '-title' in self._slides:
      idx = 1
    else:
      idx = 0

    # Add the content(s) slides
    for i in range(n):
      if i > 0:
        contents_title = ''
      self._createSlide(contents_title, name = self.name() + '-contents-' + str(i), show_in_contents=False, index=idx)
      idx += 1


  ##
  # Update the table-of-contents slide
  def contents(self):

    # Do nothing if the 'contents' flag is not set in the input file
    if not self.isParamValid('contents'):
      return

    # Loop through each object and append the markdown with title slides
    max_per_slide = int(self.getParam('contents_items_per_slide'))
    output = []
    count = 0
    page = 0

    for obj in self._warehouse.objects:
      if obj != self:
        for key in obj._slide_order:
          slide = obj._slides[key]
          if slide.getParam('title') and obj.getParam('show_in_contents'):

            if len(output) <= page:
              output.append('')

            link = '<a href="#' + slide.name() + '">'
            output[page] += '<p style="text-align:left;">' + link + obj.getParam('title') + '</a>'
            output[page] += '<span style="float:right;">' + link + str(slide.index) + '</a>'
            output[page] += '</span></p>\n'

            count += 1
            if (count >= max_per_slide):
              count = 0
              page += 1

    for i in range(len(output)):
      contents_name = self.name() + '-contents-' + str(i)
      self._slides[contents_name].markdown += output[i]
