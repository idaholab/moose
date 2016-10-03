[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 200
  ny = 10
  xmax = 0.304 # Length of test chamber
  ymax = 0.0257 # Test chamber radius
[]

[Variables]
  [./temp]
    initial_condition = 300 # Start at room temperature
  [../]
[]

[AuxVariables]
  [./pressure]
    initial_condition = 10000
  [../]
[]

[Kernels]
  [./heat_conduction]
    type = HeatConduction
    variable = temp
  [../]
  [./heat_conduction_time_derivative]
    type = HeatCapacityConductionTimeDerivative
    variable = temp
  [../]
  [./heat_convection]
    type = DarcyConvection
    variable = temp
    darcy_pressure = pressure
  [../]
[]

[BCs]
  [./inlet_temperature]
    type = DirichletBC
    variable = temp
    boundary = left
    value = 350 # (C)
  [../]
  [./outlet_temperature]
    type = HeatConductionOutflow
    variable = temp
    boundary = right
  [../]
[]

[Materials]
  [./column]
    type = PackedColumn
    block = 0
    ball_radius = 1
  [../]
[]

[Problem]
  type = FEProblem
  coord_type = RZ
  rz_coord_axis = X
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
