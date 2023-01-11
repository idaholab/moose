[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  elem_type = QUAD4
  nx = 4
  ny = 4
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./force]
    type = BodyForce
    variable = u
    function = time_function
  [../]
[]

[Functions]
  [./time_function]
    type = ParsedFunction
    expression = 't+1'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 'left right bottom top'
    value = 0
  [../]
[]

[Postprocessors]
  [./norm]
    type = ElementL2Norm
    variable = u
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
