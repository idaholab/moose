[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
[]

[Variables][dummy][][]

[Problem]
  kernel_coverage_check = false
[]

[AuxVariables]
  [uniform]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Postprocessors]
  [m]
    type = PointValue
    variable = uniform
    point = '0.25 0.25 0.0'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
