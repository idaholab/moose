#
# Test the gradient calculation in spline function and the gradient pass-through in FunctionIC
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 3
  ymin = 0
  ymax = 1
  nx = 10
  ny = 2
[]

[Variables]
  [./u]
    order = THIRD
    family = HERMITE
  [../]
[]

[Functions]
  [./spline_function]
    type = SplineFunction
    x = '0 1 2 3'
    y = '0 1 0 1'
  [../]
[]

[ICs]
  [./u_ic]
    type = FunctionIC
    variable = 'u'
    function = spline_function
  [../]
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  file_base = spline
  [./OverSampling]
    type = Exodus
    refinements = 3
  [../]
[]
