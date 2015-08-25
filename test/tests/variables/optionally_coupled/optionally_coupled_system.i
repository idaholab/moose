[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  elem_type = QUAD4
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./coupled]
    type = OptionallyCoupledForce
    variable = u
    v = v
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]


  [./left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 2
  [../]
  [./right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 3
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
