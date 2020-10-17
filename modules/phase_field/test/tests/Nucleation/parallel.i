[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
  nz = 0
  xmin = 0
  xmax = 20
  ymin = 0
  ymax = 20
[]

[GlobalParams]
  derivative_order = 2
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
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
  [../]
[]

[UserObjects]
  [./inserter]
    type = DiscreteNucleationInserter
    hold_time = 1
    probability = 0.01
    radius = 4
  [../]
  [./map]
    type = DiscreteNucleationMap
    periodic = c
    inserter = inserter
  [../]
[]

[Postprocessors]
  [./sum]
    type = ElementIntegralMaterialProperty
    mat_prop = F
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  num_steps = 10
  dt = 0.1
[]

[Outputs]
  execute_on = 'timestep_end'
  [./out]
    type = CSV
  [../]
[]
