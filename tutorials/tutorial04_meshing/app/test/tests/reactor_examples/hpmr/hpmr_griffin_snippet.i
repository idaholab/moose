### This file contains a snippet from a Griffin input file. This file is not runnable in this form. ###

[Materials]
  [mat1]
    type = CoupledFeedbackNeutronicsMaterial
    block = '250'
    material_id = 1
  []
  [mat2]
    type = CoupledFeedbackNeutronicsMaterial
    block = '600'
    material_id = 2
  []
  # Repeat for all other blocks in mesh
[]
