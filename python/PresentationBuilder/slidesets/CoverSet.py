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
    params.addParam("active", "A list of active slide sets")
    params.addParam("inactive", "A list of inactive slide sets")
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
    max_per_slide = float(self.getParam('contents_items_per_slide'))
    n = int(math.ceil(cnt / max_per_slide))
    self._createContentsSlides(n)


  ##
  # Update the table-of-contents slide
  def contents(self):

    # Define the active/inactive lists
    if self.isParamValid("active"):
      self.__active = self.getParam("active")
    else:
      for obj in self.__warehouse.objects:
        self.__active += ' ' + obj.name()

    if self.isParamValid("inactive"):
      self.__inactive = self.getParam("inactive")

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

        # Do nothing if the slide is not active
        if not self.__isSetActive(obj):
          continue

        # Do nothing if a title slide does not exist
        title_name = obj.name() + '-title'
        if not obj.warehouse().hasObject(title_name):
          continue

        slide = obj.warehouse().getObject(title_name)
        if len(output) <= page:
          output.append('')

        link = '<a href="#' + slide.name() + '">'
        output[page] += '<p style="text-align:left;line-height:12px">' + link + slide.title()[1] + '</a>'
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
  def __isSetActive(self, obj):
    if (obj.name() not in self.__active) or (obj.name() in self.__inactive):
      return False
    return True
