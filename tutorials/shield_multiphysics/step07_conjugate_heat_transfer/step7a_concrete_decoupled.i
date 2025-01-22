cp_multiplier = 1e-6

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = '../step03_boundary_conditions/mesh_in.e'
  []
[]

[Variables]
  [T]
    # Adds a Linear Lagrange variable by default
    block = 'concrete concrete_and_Al'
  []
[]

[AuxVariables]
  [velocity]
    family = LAGRANGE_VEC
    block = 'water'
  []
  [p]
    block = 'water'
  []
  [T_water]
    initial_condition = 300
    block = 'water'
  []
[]

[ICs]
  [velocity]
    type = VectorConstantIC
    x_value = 1e-15
    y_value = 1e-15
    z_value = 1e-15
    variable = velocity
  []
[]

[Kernels]
  # Solid heat conduction
  [diffusion_concrete]
    type = ADHeatConduction
    variable = T
  []
  [time_derivative]
    type = ADHeatConductionTimeDerivative
    variable = T
  []
[]

[Materials]
  [concrete]
    type = ADHeatConductionMaterial
    block = 'concrete'
    temp = 'T'
    # we specify a function of time, temperature is passed as the time argument
    # in the material
    thermal_conductivity_temperature_function = '2.25 + 0.001 * t'
    specific_heat = '${fparse cp_multiplier * 1170}'
  []
  [concrete_and_Al]
    type = ADHeatConductionMaterial
    block = 'concrete_and_Al'
    temp = 'T'
    # Al: 175 W/m/K, concrete: 2.5 W/m/K
    thermal_conductivity_temperature_function = '45'
    specific_heat = '1170'
  []
  [density]
    type = ADGenericConstantMaterial
    block = 'concrete concrete_and_Al'
    prop_names = 'density'
    prop_values = '2400' # kg / m3
  []
[]

[BCs]
  # Solid
  [from_reactor]
    type = FunctionNeumannBC
    variable = T
    boundary = inner_cavity
    # 100 kW reactor, 108 m2 cavity area
    # ramp up over 10s
    function = '1e5 / 108 * min(t / 10, 1)'
  []
  [air_convection]
    type = ADConvectiveHeatFluxBC
    variable = T
    boundary = 'air_boundary'
    T_infinity = 300.0
    # The heat transfer coefficient should be obtained from a correlation
    heat_transfer_coefficient = 10
  []
  [ground]
    type = DirichletBC
    variable = T
    value = 300
    boundary = 'ground'
  []

  # Heat fluxes for decoupling
  [water_convection]
    type = ADConvectiveHeatFluxBC
    variable = T
    boundary = 'water_boundary_inwards'
    T_infinity = 300.0
    # The heat transfer coefficient should be obtained from a correlation
    heat_transfer_coefficient = 30
  []
[]

[Problem]
  # No kernel defined in water yet
  kernel_coverage_check = false
  # No material defined in water yet
  material_coverage_check = false
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  automatic_scaling = true

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  l_abs_tol = 1e-9
  nl_abs_tol = 1e-8

  end_time = 100
  dt = 0.25
  start_time = -1

  # steady_state_tolerance = 1e-5
  # steady_state_detection = true

  [TimeStepper]
    type = FunctionDT
    function = 'if(t<0,0.1,0.25)'
  []
[]

[Outputs]
  exodus = true
[]
