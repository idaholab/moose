[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 200
    ny = 10
    xmax = 0.304 # Length of test chamber
    ymax = 0.0257 # Test chamber radius
  []
  coord_type = RZ
  rz_coord_axis = X
[]

[Variables]
  [temperature]
    initial_condition = 300 # Start at room temperature
  []
[]

[AuxVariables]
  [pressure]
  []
[]

[AuxKernels]
  [pressure]
    type = FunctionAux
    variable = pressure
    function = '4000 - 3000 * x - 3000 * t*x*x*y'
    execute_on = timestep_end
  []
[]

[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
  []
  [heat_conduction_time_derivative]
    type = ADHeatConductionTimeDerivative
    variable = temperature
  []
  [heat_convection]
    type = DarcyAdvection
    variable = temperature
    pressure = pressure
  []
[]

[BCs]
  [inlet_temperature]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 350
  []
  [outlet_temperature]
    type = HeatConductionOutflow
    variable = temperature
    boundary = right
  []
[]

[Materials]
  [column]
    type = PackedColumn
    radius = 1
    temperature = 293.15 # 20C
  []
[]

[Problem]
  type = FEProblem
[]

[Executioner]
  type = Transient
  num_steps = 300
  dt = 0.1
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
