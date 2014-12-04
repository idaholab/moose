import os, re, urllib, urlparse
from ..slidesets import DjangoWikiSet, INLSetInterface

##
# A Django wiki set of slids formated in INL presentation style
class INLDjangoWikiSet(DjangoWikiSet, INLSetInterface):

  ##
  # Valid parameters for the WikiSet class
  @staticmethod
  def validParams():
    params = DjangoWikiSet.validParams()
    params += INLSetInterface.validParams()
    return params

  ##
  # Initialize contents for INL slides
  # This creates injects the dark title slide and centers the title
  def initContents(self):
    DjangoWikiSet.initContents(self)   # base class initialization
    INLSetInterface.initContents(self) # format the contents for INL slides
