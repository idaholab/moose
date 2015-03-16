from FactorySystem import Warehouse

##
# A warehouse for storing slide objects
class SlideWarehouse(Warehouse):

  ##
  # Constructor
  def __init__(self, **kwargs):
    Warehouse.__init__(self)

    # The name of the set object that owns this list
    self.__slide_set_name = kwargs.pop('set_name')

    # An active/inactive list for which slides to display
    self.__active = kwargs.pop('active')
    self.__inactive = kwargs.pop('inactive')

    # The slide order for output by activeObjects
    self.__slide_order = []

    # Map between the slide name and location in the objects list
    self.__slide_map = dict()

    # Object counter
    self.__count = 0


  ##
  # Add a new object to the warehouse, in a specific location
  # @param index The location to insert the object
  # @param moose_object The object to add
  def insertObject(self, index, moose_object):
    Warehouse.addObject(self, moose_object)
    name = moose_object.name()
    self.__slide_map[name] = self.__count
    self.__slide_order.insert(index, name)
    self.__count += 1


  ##
  # Add a new object to the warehouse
  # @param moose_object The object to add
  def addObject(self, moose_object):
    Warehouse.addObject(self, moose_object)
    name = moose_object.name()
    self.__slide_order.append(name)
    self.__slide_map[name] = self.__count
    self.__count += 1


  ##
  # The number of objects
  def numObjects(self):
    return len(self.objects)


  ##
  # Retrieves an object by name
  # @param name The name of the slide to retrieve
  def getObject(self, name):
    idx = self.__slide_map[name]
    return self.objects[idx]

  ##
  # Returns true of the supplied name exists in the warehouse
  # @param name The name of the slide to check existence
  def hasObject(self, name):
    return name in self.__slide_map

  ##
  # Return the active slide objects (public)
  def activeObjects(self):

    slides = []

    for name in self.__slide_order:
      slide = self.objects[self.__slide_map[name]]
      active = False

      # Determine active status from active list,
      # if the active list does not exist then everything is
      # active
      if not self.__active:
        active = True
      else:
        for active_name in self.__active:
          if active_name in slide.name():
            active = True
            break

      # Contents and titles are assumed active
      if slide.name().endswith('-contents') or slide.name().endswith('-title'):
        active = True

      # Handle inactive
      for inactive_name in self.__inactive:
        if inactive_name in slide.name():
          active = False
          break

      # Append the slide if active
      if active:
        slides.append(slide)


    return slides
