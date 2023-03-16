[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 30
    ny = 3
    xmax = 0.304 # Length of test chamber
    ymax = 0.0257 # Test chamber radius
  []
  coord_type = RZ
  rz_coord_axis = X
  uniform_refine = 2
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
  [column]
    type = PackedColumn
    radius = 1
    temperature = temperature
    porosity = '0.25952 + 0.7*y/0.0257'
  []
[]

[Postprocessors]
  [average_temperature]
    type = ElementAverageValue
    variable = temperature
  []
  [outlet_heat_flux]
    type = ADSideDiffusiveFluxIntegral
    variable = temperature
    boundary = right
    diffusivity = thermal_conductivity
  []
[]

[VectorPostprocessors]
  [temperature_sample]
    type = LineValueSampler
    num_points = 500
    start_point = '0.1 0      0'
    end_point =   '0.1 0.0257 0'
    variable = temperature
    sort_by = y
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

[Outputs]
  exodus = true
  csv = true
[]
