[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
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
[]


[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 2
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 3
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  nl_rel_tol = 1e-10
[]

[Outputs]
  exodus = true
  xda = true
[]
