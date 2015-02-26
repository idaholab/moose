import os, sys
from FactorySystem import InputParameters
from ..slidesets import SlideSet

class INLSetInterface():

  ##
  # Create the valid parameters for the INL slide set
  # These parameters should be added to the base class parameters using the += operator
  #
  # @see INLCoverSet
  @staticmethod
  def validParams():
    params = InputParameters()
    params.addParam('background-image', 'inl_white_slide.png', 'Slide background image for INL slide set')
    return params

  ##
  # Initialize the contents for the INL presentations
  # This method should be called after the initContents method of the base class
  #
  # @see initContents
  def initContents(self):

    # If a contents slide exists, set it up for INL presentation style
    contents_slide_name = self.name() + '-title'
    if self.warehouse().hasObject(contents_slide_name):
      contents = self.warehouse().getObject(contents_slide_name)
      contents.parameters()['background-image'] = 'inl_dark_title.png'
      contents.parameters()['class'] = 'middle, cover'
