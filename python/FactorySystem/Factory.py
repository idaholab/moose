#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, sys
import inspect
import json
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


    def loadPlugins(self, base_dirs, plugin_path, attribute):
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
                        # Search through the module and look for classes that
                        # have the passed in attribute, which should be a bool and be True
                        for name, obj in inspect.getmembers(sys.modules[module_name]):
                            if inspect.isclass(obj) and hasattr(obj, attribute):
                                at = getattr(obj, attribute)
                                if isinstance(at, bool) and at:
                                    self.register(obj, name)
                    except Exception as e:
                        print('\nERROR: Your Plugin Tester "' + module_name + '" failed to import. (skipping)\n\n' + str(e))


    def printDump(self, root_node_name):
        print("[" + root_node_name + "]")

        for name, object in sorted(self.objects.items()):
            print("  [./" + name + "]")
            params = self.validParams(name)
            for key in sorted(params.desc):
                default = ''
                if params.isValid(key):
                    the_param = params[key]
                    if type(the_param) == list:
                        default = "'" + " ".join(the_param) + "'"
                    else:
                        default = str(the_param)

                print("%4s%-30s = %-30s # %s" % ('', key, default, params.getDescription(key)))
            print("  [../]\n")
        print("[]")

    def dumpJSON(self, root_node_name):
        with open('../test/testoutput.json', 'w') as file:
            out = {}
            for name, object in sorted(self.objects.items()):
                out[name] = {}
                params = self.validParams(name)
                for key in sorted(params.desc):
                    if params.isValid(key):
                        out[name][key] = {"default" : params[key], "description" : params.getDescription(key)}

            file.write(json.dumps(out, indent=4, sort_keys=True))
            print("JSON dumped to moose/test/testoutput.json")
