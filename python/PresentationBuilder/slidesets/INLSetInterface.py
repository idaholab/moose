import os, sys
from FactorySystem import InputParameters

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
  # Parsing that sets up INL cover design
  # This method should be called before the _parseSlide method of the base class
  # @see INLCoverSet, INLDjangoWikiSet, INLMergeCoverSet
  @staticmethod
  def applySlideSettings(slide):
    if slide.name():
      if slide.name().endswith('-title'):
        slide.parameters()['background-image'] = 'inl_dark_title.png'
        slide.parameters()['class'] = 'middle, cover'
      elif slide.name().endswith('-subtitle'):
        slide.parameters()['class'] = 'middle, center'
