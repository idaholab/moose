# A non-linear and aux variable with the same name

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 5
  ny = 5
  elem_type = QUAD4
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Executioner]
  type = Transient
[]
