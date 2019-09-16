[Mesh]
  type = GeneratedMesh
  elem_type = HEX27
  dim = 3
[]

[Variables]
  [u]
    order = SECOND
  []
  [v]
    order = SECOND
  []
[]

[Kernels]
  [u_diff]
    type = ADDiffusion
    variable = u
  []
  [v_diff]
    type = ADDiffusion
    variable = v
  []
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady
[]
