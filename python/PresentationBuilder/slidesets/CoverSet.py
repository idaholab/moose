# Import the SlideSet base class
import math
from ..slidesets import RemarkSlideSet

##
# A special set of slides for creating cover page and contents
class CoverSet(RemarkSlideSet):

  ##
  # Extract the valid parameters for this object
  @staticmethod
  def validParams():
    params = RemarkSlideSet.validParams()
    params.addParam("active_sets", "A list of active slide sets")
    params.addParam("inactive_sets", "A list of inactive slide sets")
    return params


  ##
  # Constructor
  def __init__(self, name, params, **kwags):
    RemarkSlideSet.__init__(self, name, params, **kwags)

    # Need the SlideSetWarehouse
    self.__warehouse = self.getParam('_warehouse')

    # Create the list of active slides
    self.__active = ''
    self.__inactive = ''
    if self.isParamValid("active_sets"):
      self.active = self.getParam("active_sets")
    if self.isParamValid("inactive_sets"):
      self.inactive = self.getParam("inactive_sets")


  ##
  # Initializes the contents
  def initContents(self):

    # Do nothing if the 'contents' flag is not set in the input file
    if not self.isParamValid('contents') or not self.getParam('contents'):
      return

    # Count the number of contents entries
    cnt = 0
    for obj in self.__warehouse.objects:
      if obj != self and obj.getParam('show_in_contents'):
        title_name = obj.name() + '-title'
        if obj.warehouse().hasObject(title_name):
          cnt += 1

    # Create the empty contents slides
    self._createContentsSlides(cnt)


  ##
  # Update the table-of-contents slide
  def contents(self):

    # Do nothing if the 'contents' flag is not set in the input file
    if not self.isParamValid('contents') or not self.getParam('contents'):
      return

    # Loop through each object and append the markdown with title slides
    max_per_slide = int(self.getParam('contents_items_per_slide'))
    output = []
    count = 0
    page = 0

    for obj in self.__warehouse.objects:
      if obj != self and obj.getParam('show_in_contents'):
        title_name = obj.name() + '-title'
        if obj.warehouse().hasObject(title_name):
          slide = obj.warehouse().getObject(title_name)

          if not self.__isSetActive(slide):
            continue

          if len(output) <= page:
            output.append('')

          link = '<a href="#' + slide.name() + '">'
          output[page] += '<p style="text-align:left;">' + link + slide.title()[1] + '</a>'
          output[page] += '<span style="float:right;">' + link + str(slide.number) + '</a>'
          output[page] += '</span></p>\n'

          count += 1
          if (count >= max_per_slide):
            count = 0
            page += 1

    for i in range(len(output)):
      name = '-'.join([self.name(), 'contents', str(i)])
      self.warehouse().getObject(name).markdown += output[i]


  ##
  # Return true if the slide set is active for contents
  def __isSetActive(self, slide):
    parent = slide.getParam('_parent')
    if parent.name() in self.__inactive or (self.__active is not '' and parent.name() not in self.__active):
      return False
    return True
