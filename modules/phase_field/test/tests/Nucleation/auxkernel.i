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

[AuxVariables]
  [./c]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./c]
    type = DiscreteNucleationAux
    map = map
    variable = c
    no_nucleus_value = -1
    nucleus_value = 2
    execute_on = TIMESTEP_END
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
  num_steps = 10
  dt = 0.2
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
