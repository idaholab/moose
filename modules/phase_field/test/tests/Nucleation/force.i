[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  nz = 0
  xmin = 0
  xmax = 20
  ymin = 0
  ymax = 20
  elem_type = QUAD4
[]

[Variables]
  [./c]
  [../]
[]

[Kernels]
  [./c_force]
    type = DiscreteNucleationForce
    variable = c
    map = map
    no_nucleus_value = -1
    nucleus_value = 2
  [../]
  [./c_react]
    type = Reaction
    variable = c
  [../]
[]

[UserObjects]
  [./inserter]
    type = DiscreteNucleationInserter
    hold_time = 0.4
    probability = 0.01
    seed = 12346
    radius = 3.27
  [../]
  [./map]
    type = DiscreteNucleationMap
    int_width = 2
    periodic = c
    inserter = inserter
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  nl_abs_tol = 1e-10
  num_steps = 10
  dt = 0.2
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
