#!/usr/bin/python

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
