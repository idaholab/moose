[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'step13a_base_calc_out.e'
    use_for_exodus_restart = true
  []
[]

[Variables]
  [T]
    # Adds a Linear Lagrange variable by default
    block = 'concrete_hd concrete Al'
    initial_from_file_var = 'T'
  []
[]

[Kernels]
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
  [concrete_hd]
    type = ADHeatConductionMaterial
    block = concrete_hd
    temp = 'T'
    # we specify a function of time, temperature is passed as the time argument
    # in the material
    thermal_conductivity_temperature_function = '5.0 + 0.001 * t'
    specific_heat = 1050
  []
  [concrete]
    type = ADHeatConductionMaterial
    block = concrete
    temp = 'T'
    thermal_conductivity_temperature_function = '2.25 + 0.001 * t'
    specific_heat = 1050
  []
  [Al]
    type = ADHeatConductionMaterial
    block = Al
    temp = T
    thermal_conductivity_temperature_function = '175'
    specific_heat = 875
  []
  [density_concrete_hd]
    type = ADGenericConstantMaterial
    block = 'concrete_hd'
    prop_names = 'density'
    prop_values = '3524' # kg / m3
  []
  [density_concrete]
    type = ADGenericConstantMaterial
    block = 'concrete'
    prop_names = 'density'
    prop_values = '2403' # kg / m3
  []
  [density_Al]
    type = ADGenericConstantMaterial
    block = 'Al'
    prop_names = 'density'
    prop_values = '2270' # kg / m3
  []
[]

[BCs]
  [from_reactor]
    type = NeumannBC
    variable = T
    boundary = inner_cavity_solid
    # 5 MW reactor, only 50 kW removed from radiation, 144 m2 cavity area
    value = '${fparse 5e4 / 144}'
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
  [water_convection]
    type = ADConvectiveHeatFluxBC
    variable = T
    boundary = 'water_boundary_inwards'
    T_infinity = 300.0
    # The heat transfer coefficient should be obtained from a correlation
    heat_transfer_coefficient = 600
  []
[]

[Problem]
  # No kernels on the water domain
  kernel_coverage_check = false
  # No materials on the water domain
  material_coverage_check = false
[]

[Executioner]
  type = Transient
  start_time = '${units 2 day -> s}'
  num_steps = 6
  dt = '${units 12 h -> s}'
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
