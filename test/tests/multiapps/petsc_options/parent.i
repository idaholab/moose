[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./td]
    type = TimeDerivative
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
  type = Transient
  num_steps = 2
  dt = 1
  l_max_its = 4
  nl_max_its = 2

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu mumps'
  petsc_options = '-test'
  l_tol = 1e-12
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [./sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    execute_on = timestep_end
    positions = '0 0 0'
    input_files = sub.i
  [../]
[]
