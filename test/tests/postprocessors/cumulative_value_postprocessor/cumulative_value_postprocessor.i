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
  [./time_derivative]
    type = TimeDerivative
    variable = u
  [../]
  [./diff]
    type = Diffusion
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

[Problem]
  type = FEProblem
  coord_type = RZ
  rz_coord_axis = X
[]

[Executioner]
  type = Transient
  scheme = implicit-euler

  [./TimeStepper]
    type = ConstantDT
    dt = 0.01
  [../]

  start_time = 0.0
  num_steps = 2

  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Postprocessors]
  [./nonlin_it]
    type = NumNonlinearIterations
  [../]
  [./cumulative_nonlin_it]
    type = CumulativeValuePostprocessor
    postprocessor = nonlin_it
  [../]
[]

[Outputs]
  csv = true
[]
