[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'mesh2d_in.e'
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
  # Solid heat conduction
  [diffusion_concrete]
    type = ADHeatConduction
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
  []
  [concrete]
    type = ADHeatConductionMaterial
    block = concrete
    temp = 'T'
    thermal_conductivity_temperature_function = '2.25 + 0.001 * t'
  []
  [Al]
    type = ADHeatConductionMaterial
    block = Al
    temp = T
    thermal_conductivity_temperature_function = '175'
  []
[]

[BCs]
  # Solid
  [from_reactor]
    type = NeumannBC
    variable = T
    boundary = inner_cavity_solid
    # Real facility uses forced convection to cool the water tank at full power
    # Need to lower power for natural convection so water doesn't boil.
    value = '${fparse 5e4 / 14 * 0.06}'
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
    heat_transfer_coefficient = 600
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
  steady_state_detection = true
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_abs_tol = 1e-8
[]

[Outputs]
  exodus = true
[]
