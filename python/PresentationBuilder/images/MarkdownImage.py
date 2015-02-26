import os, re
from ..images import ImageBase

##
# Image class for standard markdown image blocks
class MarkdownImage(ImageBase):

  @staticmethod
  def validParams():
    params = ImageBase.validParams()
    return params

  ##
  # Constructor
  def __init__(self, name, params):
    ImageBase.__init__(self, name, params)

  ##
  #
  @staticmethod
  def match(markdown):
    output = []
    for m in re.finditer(r'\!\[(.*?)\]\(([^\s]*)\s*(.*)\)', markdown):
      output.append({'markdown' : m.group(0), \
                     'caption' : m.group(1), \
                     'url' : m.group(2), \
                     'settings' : m.group(3), \
                     'name' : m.group(2)})
    return output
