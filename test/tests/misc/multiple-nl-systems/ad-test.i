[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
[]

[Problem]
  nl_sys_names = 'u v'
[]

[Variables]
  [u]
    solver_sys = 'u'
  []
  [v]
    solver_sys = 'v'
  []
[]

[Kernels]
  [diff_u]
    type = ADDiffusion
    variable = u
  []
  [diff_v]
    type = ADDiffusion
    variable = v
  []
  [force]
    type = ADCoupledForce
    variable = v
    v = u
  []
[]

[BCs]
  [left_u]
    type = ADDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right_u]
    type = ADDirichletBC
    variable = u
    boundary = right
    value = 1
  []
  [left_v]
    type = ADDirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [right_v]
    type = ADDirichletBC
    variable = v
    boundary = right
    value = 1
  []
[]

[Preconditioning]
  [u]
    nl_sys = 'u'
    type = SMP
    petsc_options = '-snes_monitor'
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = 'hypre boomeramg'
  []
  [v]
    nl_sys = 'v'
    type = SMP
    petsc_options = '-snes_monitor'
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = 'hypre boomeramg'
  []
[]

[Executioner]
  type = SteadySolve2
  solve_type = 'NEWTON'
  first_nl_sys_to_solve = 'u'
  second_nl_sys_to_solve = 'v'
[]

[Outputs]
  print_nonlinear_residuals = false
  print_linear_residuals = false
  exodus = true
[]
