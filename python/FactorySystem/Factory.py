import os, sys, types

class Factory:
    def __init__(self):
        self.objects = {}   # The registered Objects array


    def register(self, type, name):
        self.objects[name] = type


    def validParams(self, type):
        return self.objects[type].validParams()


    def create(self, type, *args, **kwargs):
        return self.objects[type](*args, **kwargs)


    def getClassHierarchy(self, classes):
        if classes != None:
            for aclass in classes:
                classes.extend(self.getClassHierarchy(aclass.__subclasses__()))
        return classes


    def loadPlugins(self, base_dirs, plugin_path, module):
        for dir in base_dirs:
            dir = os.path.join(dir, plugin_path)
            if not os.path.exists(dir):
                continue

            sys.path.append(os.path.abspath(dir))
            for file in os.listdir(dir):
                if file[-2:] == 'py':
                    module_name = file[:-3]
                    try:
                        __import__(module_name)
                    except:
                        print '\nERROR: Your Plugin Tester "' + module_name + '" failed to import. (skipping)\n\n'

        classes = self.getClassHierarchy(module.__subclasses__())
        for aclass in classes:
            self.register(aclass, aclass.__name__)


    def printDump(self, root_node_name):
        print "[" + root_node_name + "]"

        for name, object in self.objects.iteritems():
            print "  [./" + name + "]"

            params = self.validParams(name)

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


    def printYaml(self, root_node_name):
        print "**START YAML DATA**"
        print "- name: /" + root_node_name
        print "  description: !!str"
        print "  type:"
        print "  parameters:"
        print "  subblocks:"

        for name, object in self.objects.iteritems():
            print "  - name: /" + root_node_name + "/ + name"
            print "    description:"
            print "    type:"
            print "    parameters:"

            params = self.validParams(name)
            for key in params.valid:
                required = 'No'
                if params.isRequired(key):
                    required = 'Yes'
                default = ''
                if params.isValid(key):
                    default = str(params[key])

                print "    - name: " + key
                print "      required: " + required
                print "      default: !!str " + default
                print "      description: |"
                print "        " + params.getDescription(key)

        print "**END YAML DATA**"
