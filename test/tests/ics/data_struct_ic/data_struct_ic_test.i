[Mesh]
  type = GeneratedMesh
  nx = 10
  ny = 10
  dim = 2

  # DataStructIC creates an IC based on node numbering
  parallel_type = replicated
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

[ICs]
  [./ds_ic]
    type = DataStructIC
    variable = u
  [../]
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

  solve_type = 'PJFNK'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
