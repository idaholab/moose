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
    block = 'concrete_hd concrete Al'
    initial_condition = 300
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
    specific_heat = ${fparse cp_multiplier * 1050}
  []
  [concrete]
    type = ADHeatConductionMaterial
    block = concrete
    temp = 'T'
    thermal_conductivity_temperature_function = '2.25 + 0.001 * t'
    specific_heat = ${fparse cp_multiplier * 1050}
  []
  [Al]
    type = ADHeatConductionMaterial
    block = Al
    temp = T
    thermal_conductivity_temperature_function = '175'
    specific_heat = ${fparse cp_multiplier * 875}
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

[AuxVariables]
  [heat_flux_x]
    family = MONOMIAL
    order = CONSTANT
    block = 'concrete_hd concrete'
  []
  [heat_flux_y]
    family = MONOMIAL
    order = CONSTANT
    block = 'concrete_hd concrete'
  []
  [heat_flux_z]
    family = MONOMIAL
    order = CONSTANT
    block = 'concrete_hd concrete'
  []
[]

[AuxKernels]
  [diff_flux_x]
    type = DiffusionFluxAux
    variable = heat_flux_x
    diffusion_variable = T
    diffusivity = 'thermal_conductivity'
    component = 'x'
  []
  [diff_flux_y]
    type = DiffusionFluxAux
    variable = heat_flux_x
    diffusion_variable = T
    diffusivity = 'thermal_conductivity'
    component = 'y'
  []
  [diff_flux_z]
    type = DiffusionFluxAux
    variable = heat_flux_x
    diffusion_variable = T
    diffusivity = 'thermal_conductivity'
    component = 'z'
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
  type = FEProblem
  # No kernels on the water domain
  kernel_coverage_check = false
  material_coverage_check = false
[]

[Executioner]
  type = Transient
  steady_state_detection = true
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_abs_tol = 1e-8
[]

[Outputs]
  exodus = true
[]
