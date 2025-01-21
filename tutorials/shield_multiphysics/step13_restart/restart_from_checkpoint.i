cp_multiplier = 1e-6

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'base_calc_out_cp/LATEST'
  []
[]

[Problem]
  # all variables, both nonlinear and auxiliary, are 'restarted'
  restart_file_base = 'base_calc_out_cp/LATEST'
[]


[Variables]
  [T]
    # Adds a Linear Lagrange variable by default
    block = 'concrete'
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
  [concrete]
    type = ADHeatConductionMaterial
    block = 'concrete'
    temp = 'T'
    # we specify a function of time, temperature is passed as the time argument
    # in the material
    thermal_conductivity_temperature_function = '2.25 + 0.001 * t'
    specific_heat = '${fparse cp_multiplier * 1170}'
  []
  [density]
    type = ADGenericConstantMaterial
    block = 'concrete'
    prop_names = 'density'
    prop_values = '2400' # kg / m3
  []
[]

[AuxVariables]
  [T_water]
    block = 'water'
  []
  [heat_flux_x]
    family = MONOMIAL
    order = CONSTANT
    block = 'concrete'
  []
  [heat_flux_y]
    family = MONOMIAL
    order = CONSTANT
    block = 'concrete'
  []
  [heat_flux_z]
    family = MONOMIAL
    order = CONSTANT
    block = 'concrete'
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
  [water_convection]
    type = ADConvectiveHeatFluxBC
    variable = T
    # the sideset needs to point from concrete to water
    boundary = 'water_boundary_inwards'
    T_infinity = 300.0
    # The heat transfer coefficient should be obtained from a correlation
    heat_transfer_coefficient = 30
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
  # 10 time steps from the start time of the checkpoint
  num_steps = 10
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
