[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 15
  ny = 15
[]

[Variables]
  [./u]
    scaling = 1e-5
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    preset = false
    boundary = left
    value = -1000
  [../]
  [./right]
    type = DirichletBC
    variable = u
    preset = false
    boundary = right
    value = 100000
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'implicit-euler'
  line_search = 'none'
  solve_type = PJFNK

  l_max_its = 20
  nl_max_its = 20
  nl_div_tol = 10

  dt = 1
  num_steps = 3

  petsc_options = '-snes_converged_reason -ksp_converged_reason '
  petsc_options_iname = '-pc_type -pc_hypre_type '
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
