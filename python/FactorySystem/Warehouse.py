#!/usr/bin/python
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

class Warehouse:
    def __init__(self):
        self.objects = []
        self.active = []

    def addObject(self, moose_object):
        self.objects.append(moose_object)
        self.active.append(moose_object)

    def getActiveObjects(self):
        return self.active

    def getAllObjects(self):
        return self.objects

    def markAllObjectsInactive(self):
        self.active = []

    def clear(self):
        self.objects = []
        self.active = []
