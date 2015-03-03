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
  # The unumber of objects
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

    # By default return all slide objects
    slides = []

    # Create the active list, it is assumed that contents
    # and titles should be active unless listed as inactive
    if self.__active:
      for name in self.__slide_order:
        for active in self.__active:
          if (active in name) \
             or name.startswith(self.__slide_set_name + '-contents') \
             or name.startswith(self.__slide_set_name + '-title'):
            slides.append(self.objects[self.__slide_map[name]])
    else:
      for name in self.__slide_order:
        slides.append(self.objects[self.__slide_map[name]])

    # Remove inactive slides
    if self.__inactive:
      for slide in slides:
        for inactive in self.__inactive:
          if inactive in slide.name():
            slides.remove(slide)

    return slides
