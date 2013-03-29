class Factory:
  def __init__(self):
    self.objects = {}   # The registered Objects array

  def register(self, type, name):
    self.objects[name] = type

  def getValidParams(self, type):
    return self.objects[type].getValidParams()

  def create(self, type, params):
    return self.objects[type]('object', params)

  def printDump(self, root_node_name):
    print "[" + root_node_name + "]"

    for name, object in self.objects.iteritems():
      print "  [./" + name + "]"

      params = self.getValidParams(name)

      for key in params.desc:
        required = 'No'
        if params.isRequired(key):
          required = 'Yes'
        default = ''
        if params.isValid(key):
	  the_param = params[key]
	  if type(the_param) == list:
            default = "'" + " ".join(the_param) + "'"
	  else:
	    default = str(the_param)

        print "%4s%-30s = %-30s # %s" % ('', key, default, params.getDescription(key))
      print "  [../]\n"
    print "[]"
