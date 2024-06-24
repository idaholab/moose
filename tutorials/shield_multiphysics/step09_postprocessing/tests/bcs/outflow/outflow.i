[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 30
    ny = 5
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

[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
  []
  [heat_conduction_time_derivative]
    type = ADHeatConductionTimeDerivative
    variable = temperature
  []
[]

[BCs]
  [inlet_temperature]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 350 # (K)
  []
  [outlet_temperature]
    type = HeatConductionOutflow
    variable = temperature
    boundary = right
  []
[]

[Materials]
  [steel]
    type = ADGenericConstantMaterial
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '18 466 8000' # W/m*K, J/kg-K, kg/m^3 @ 296K
  []
[]

[Problem]
  type = FEProblem
[]

[Executioner]
  type = Transient
  num_steps = 2
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
