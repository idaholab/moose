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

[Materials]
  [./nucleation]
    type = DiscreteNucleation
    op_names  = c
    op_values = 1
    map = map
    outputs = exodus
  [../]
[]

[UserObjects]
  [./inserter]
    type = DiscreteNucleationInserter
    hold_time = 1
    probability = 0.01
    radius = 3.27
  [../]
  [./map]
    type = DiscreteNucleationMap
    periodic = c
    inserter = inserter
  [../]
[]

[Adaptivity]
  [./Markers]
    [./nuc]
      type = DiscreteNucleationMarker
      map = map
    [../]
  [../]
  marker = nuc
  cycles_per_step = 3
  recompute_markers_during_cycles = true
  max_h_level = 3
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  num_steps = 10
  dt = 0.1
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  hide = c
[]
