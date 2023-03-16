[Mesh]
  uniform_refine = 3
  [generate]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 40
    ny = 4
    xmax = 0.304 # Length of test chamber
    ymax = 0.0257 # Test chamber radius
  []
  [bottom]
    type = SubdomainBoundingBoxGenerator
    input = generate
    location = inside
    bottom_left = '0 0 0'
    top_right = '0.304 0.01285 0'
    block_id = 1
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

[BCs]
  [inlet]
    type = DirichletBC
    variable = pressure
    boundary = left
    value = 4000 # (Pa) From Figure 2 from paper.  First data point for 1mm spheres.
  []
  [outlet]
    type = DirichletBC
    variable = pressure
    boundary = right
    value = 0 # (Pa) Gives the correct pressure drop from Figure 2 for 1mm spheres
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
  viscosity_file = data/water_viscosity.csv
  density_file = data/water_density.csv
  thermal_conductivity_file = data/water_thermal_conductivity.csv
  specific_heat_file = data/water_specific_heat.csv

  [column_bottom]
    type = PackedColumn
    block = 1
    radius = 1.15
    temperature = temperature
    fluid_viscosity_file = ${viscosity_file}
    fluid_density_file = ${density_file}
    fluid_thermal_conductivity_file = ${thermal_conductivity_file}
    fluid_specific_heat_file = ${specific_heat_file}
  []
  [column_top]
    type = PackedColumn
    block = 0
    radius = 1
    temperature = temperature
    porosity = '0.25952 + 0.7*x/0.304'
    fluid_viscosity_file = ${viscosity_file}
    fluid_density_file = ${density_file}
    fluid_thermal_conductivity_file = ${thermal_conductivity_file}
    fluid_specific_heat_file = ${specific_heat_file}
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
    function = 'if(t<0,0.1,0.25)'
  []
[]

[Adaptivity]
  marker = error_frac
  max_h_level = 3
  [Indicators]
    [temperature_jump]
      type = GradientJumpIndicator
      variable = temperature
      scale_by_flux_faces = true
    []
  []
  [Markers]
    [error_frac]
      type = ErrorFractionMarker
      coarsen = 0.025
      indicator = temperature_jump
      refine = 0.9
    []
  []
[]

[Outputs]
  [out]
    type = Exodus
    output_material_properties = true
  []
[]
