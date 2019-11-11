[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 10
  xmax = 0.304 # Length of test chamber
  ymax = 0.0257 # Test chamber radius
[]

[Variables]
  [temperature]
  []
[]

[ICs]
  [temperature]
    type = FunctionIC
    function = '300 + 100*y'
    variable = temperature
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
    type = FunctionDirichletBC
    variable = temperature
    boundary = left
    function = '350 + 100*y' # (K)
  []
  [outlet_temperature]
    type = FunctionDirichletBC
    variable = temperature
    boundary = right
    function = '300 + 100*y' # (K)
  []
[]

[Materials]
  [steel]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '18 0.466 8000' # W/m*K, J/kg-K, kg/m^3 @ 296K
  []
[]

[Problem]
  type = FEProblem
  coord_type = RZ
  rz_coord_axis = X
[]

[Executioner]
  type = Transient
  num_steps = 100
  dt = 0.01
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
