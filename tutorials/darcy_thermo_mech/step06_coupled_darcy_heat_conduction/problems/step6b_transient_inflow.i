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
  [pressure]
  []
  [temperature]
    initial_condition = 300 # Start at room temperature
  []
[]

[AuxVariables]
  [velocity]
    order = CONSTANT
    family = MONOMIAL_VEC
  []
[]

[Kernels]
  [darcy_pressure]
    type = DarcyPressure
    variable = pressure
  []
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

[AuxKernels]
  [velocity]
    type = DarcyVelocity
    variable = velocity
    execute_on = timestep_end
    pressure = pressure
  []
[]

[Functions]
  [inlet_function]
    type = ParsedFunction
    expression = 2000*sin(0.466*pi*t) # Inlet signal from Fig. 3
  []
  [outlet_function]
    type = ParsedFunction
    expression = 2000*cos(0.466*pi*t) # Outlet signal from Fig. 3
  []
[]

[BCs]
  [inlet]
    type = FunctionDirichletBC
    variable = pressure
    boundary = left
    function = inlet_function
  []
  [outlet]
    type = FunctionDirichletBC
    variable = pressure
    boundary = right
    function = outlet_function
  []
  [inlet_temperature]
    type = FunctionDirichletBC
    variable = temperature
    boundary = left
    function = 'if(t<0,350+50*t,350)'
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
    temperature = temperature
    fluid_viscosity_file = data/water_viscosity.csv
    fluid_density_file = data/water_density.csv
    fluid_thermal_conductivity_file = data/water_thermal_conductivity.csv
    fluid_specific_heat_file = data/water_specific_heat.csv
    outputs = exodus
  []
[]

[Problem]
  type = FEProblem
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  automatic_scaling = true

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  end_time = 100
  dt = 0.25
  start_time = -1

  steady_state_tolerance = 1e-5
  steady_state_detection = true

  [TimeStepper]
    type = FunctionDT
    function = 'if(t<0,0.1,(2*pi/(0.466*pi))/16)' # dt to always hit the peaks of sine/cosine BC
  []
[]

[Outputs]
  exodus = true
[]
