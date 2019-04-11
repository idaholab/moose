[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 2
  xmax = 5
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
  [./dt]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 10
  [../]
  [./right]
    type = NeumannBC
    variable = u
    boundary = right
    value = -1
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dtmin = 1.0
  end_time = 25.0
  [./TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 1
    linear_iteration_ratio = 1
    dt = 2.0
  [../]
[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]

[Problem]
  restart_file_base = adapt_tstep_shrink_init_dt_out_cp/LATEST
[]
