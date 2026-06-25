[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [T][]
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = T
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = T
    boundary = bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = T
    boundary = top
    value = 1
  []
[]

[Constraints]
  [tied]
    type = TiedValueConstraint
    variable = T
    primary_variable = T
    secondary = left
    primary = right
  []
[]

[Executioner]
  type = Steady
[]

[Outputs][]
