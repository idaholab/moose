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
  start_time = 0.0
  end_time = 20.0
  n_startup_steps = 2
  dtmax = 6.0
  [./TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 10
    dt = 1.0
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
  checkpoint = true
[]
