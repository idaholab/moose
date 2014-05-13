#!/usr/bin/python

class Warehouse:
  def __init__(self):
    self.objects = []

  def addObject(self, moose_object):
    self.objects.append(moose_object)

  def getAllObjects(self):
    return self.objects
