[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = 1
  ymax = 1
  nx = 100
  ny = 100
  elem_type = QUAD9
[]

[AuxVariables/phi]
  family = LAGRANGE
  order = FIRST
[]

[Functions/phi_exact]
  type = LevelSetOlssonPlane
  epsilon = 0.04
  point = '0.5 0.5 0'
  normal = '0 1 0'
[]

[ICs/phi_ic]
  type = FunctionIC
  function = phi_exact
  variable = phi
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
