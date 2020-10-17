[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  nz = 0
  xmin = 0
  xmax = 20
  ymin = 0
  ymax = 20
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    boundary = left
    variable = c
    value = 0
  [../]
  [./right]
    type = DirichletBC
    boundary = right
    variable = c
    value = 1
  [../]
  [./Periodic]
    [./all]
      auto_direction = y
    [../]
  [../]
[]

[Kernels]
  [./c]
    type = Diffusion
    variable = c
  [../]
  [./dt]
    type = TimeDerivative
    variable = c
  [../]
[]

[UserObjects]
  [./inserter]
    type = DiscreteNucleationInserter
    hold_time = 1
    probability = 0.0005
    radius = 3.27
  [../]
  [./map]
    type = DiscreteNucleationMap
    periodic = c
    inserter = inserter
  [../]
[]

[Postprocessors]
  [./dt]
    type = TimestepSize
  [../]
  [./dtnuc]
    type = DiscreteNucleationTimeStep
    inserter = inserter
    p2nucleus = 0.1
    dt_max = 0.5
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  num_steps = 20

  [./TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 8
    iteration_window = 2
    timestep_limiting_postprocessor = dtnuc
    dt = 1
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = true
[]
