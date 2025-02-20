cp_water_multiplier = 1e-10
mu_multiplier = 1e4

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'mesh2d_in.e'
  []
[]

[Variables]
  [T_solid]
    type = INSFVEnergyVariable
    block = 'concrete_hd concrete Al'
    initial_condition = 300
    scaling = 1e-05
  []

  [vel_x]
    type = INSFVVelocityVariable
    block = 'water'
    initial_condition = 1e-4
  []
  [vel_y]
    type = INSFVVelocityVariable
    block = 'water'
    initial_condition = 1e-4
  []
  # This isn't used in simulation, but useful for visualization
  [vel_z]
    type = INSFVVelocityVariable
    block = 'water'
    initial_condition = 0
  []
  [pressure]
    type = INSFVPressureVariable
    block = 'water'
    initial_condition = 1e5
  []
  [T_fluid]
    type = INSFVEnergyVariable
    initial_condition = 300
    block = 'water'
    scaling = 1e-05
  []
  [lambda]
    type = MooseVariableScalar
    family = SCALAR
    order = FIRST
  []
[]

[GlobalParams]
  velocity_interp_method = rc
  rhie_chow_user_object = ins_rhie_chow_interpolator
  rho = rho
[]

[FVKernels]
  [solid_heat_conduction]
    type = FVDiffusion
    variable = T_solid
    coeff = 'k'
  []

  [water_ins_mass_advection]
    type = INSFVMassAdvection
    advected_interp_method = upwind
    block = water
    variable = pressure
  []
  [water_ins_mass_pressure_pin]
    type = FVPointValueConstraint
    lambda = lambda
    phi0 = 1e5
    point = '1 3 0'
    variable = pressure
  []
  [water_ins_momentum_time_vel_x]
    type = INSFVMomentumTimeDerivative
    block = water
    momentum_component = x
    variable = vel_x
  []
  [water_ins_momentum_time_vel_y]
    type = INSFVMomentumTimeDerivative
    block = water
    momentum_component = y
    variable = vel_y
  []
  [water_ins_momentum_advection_x]
    type = INSFVMomentumAdvection
    advected_interp_method = upwind
    block = water
    momentum_component = x
    variable = vel_x
  []
  [water_ins_momentum_advection_y]
    type = INSFVMomentumAdvection
    advected_interp_method = upwind
    block = water
    momentum_component = y
    variable = vel_y
  []
  [water_ins_momentum_diffusion_x]
    type = INSFVMomentumDiffusion
    block = water
    momentum_component = x
    mu = mu
    variable = vel_x
  []
  [water_ins_momentum_diffusion_y]
    type = INSFVMomentumDiffusion
    block = water
    momentum_component = y
    mu = mu
    variable = vel_y
  []
  [water_ins_momentum_pressure_x]
    type = INSFVMomentumPressure
    block = water
    momentum_component = x
    pressure = pressure
    variable = vel_x
  []
  [water_ins_momentum_pressure_y]
    type = INSFVMomentumPressure
    block = water
    momentum_component = y
    pressure = pressure
    variable = vel_y
  []
  [water_ins_momentum_gravity_z]
    type = INSFVMomentumGravity
    block = water
    gravity = '0 -9.81 0'
    momentum_component = y
    variable = vel_y
  []
  [water_ins_momentum_boussinesq_z]
    type = INSFVMomentumBoussinesq
    T_fluid = T_fluid
    alpha_name = alpha
    block = water
    gravity = '0 -9.81 0'
    momentum_component = y
    ref_temperature = 300
    rho = 955.7
    variable = vel_y
  []

  # Energy conservation equation
  [water_ins_energy_time]
    type = INSFVEnergyTimeDerivative
    block = water
    dh_dt = dh_dt
    rho = rho
    variable = T_fluid
  []
  [water_ins_energy_advection]
    type = INSFVEnergyAdvection
    advected_interp_method = upwind
    block = water
    variable = T_fluid
  []
  [water_ins_energy_diffusion_all]
    type = FVDiffusion
    block = water
    coeff = k
    variable = T_fluid
  []
[]

[FunctorMaterials]
  [concrete_hd]
    type = ADParsedFunctorMaterial
    block = 'concrete_hd'
    expression = '5.0 + 0.001 * T_solid'
    functor_names = 'T_solid'
    property_name = 'k'
  []
  [concrete]
    type = ADParsedFunctorMaterial
    block = 'concrete'
    expression = '2.25 + 0.001 * T_solid'
    functor_names = 'T_solid'
    property_name = 'k'
  []
  [Al]
    type = ADGenericFunctorMaterial
    block = 'Al'
    prop_names = 'k'
    prop_values = '175'
  []

  [water]
    type = ADGenericFunctorMaterial
    block = 'water'
    prop_names = 'rho    k     cp      mu alpha_wall'
    prop_values = '955.7 0.6 ${fparse cp_water_multiplier * 4181} ${fparse 7.98e-4 * mu_multiplier} 30'
  []
  [boussinesq_params]
    type = ADGenericFunctorMaterial
    prop_names = 'alpha '
    prop_values = '2.9e-3'
  []
  [water_ins_enthalpy_material]
    type = INSFVEnthalpyFunctorMaterial
    block = water
    cp = cp
    execute_on = ALWAYS
    outputs = none
    temperature = T_fluid
  []
[]

[FVBCs]
  [T_solid_inner_cavity]
    type = FVFunctorNeumannBC
    boundary = 'inner_cavity_solid'
    variable = T_solid
    functor = '${fparse 5e4 / 144 * 0.1}'
  []
  [T_solid_air]
    type = FVFunctorConvectiveHeatFluxBC
    boundary = 'air_boundary'
    variable = T_solid
    T_bulk = 300
    T_solid = T_solid
    heat_transfer_coefficient = 10
    is_solid = true
  []
  [T_solid_ground]
    type = FVDirichletBC
    boundary = 'ground'
    variable = T_solid
    value = 300
  []

  [vel_x_water_boundary]
    type = INSFVNoSlipWallBC
    boundary = 'water_boundary inner_cavity_water'
    function = 0
    variable = vel_x
  []
  [vel_y_water_boundary]
    type = INSFVNoSlipWallBC
    boundary = 'water_boundary inner_cavity_water'
    function = 0
    variable = vel_y
  []

  [T_fluid_inner_cavity]
    type = FVFunctorNeumannBC
    boundary = inner_cavity_water
    # Real facility uses forced convection to cool the water tank at full power
    # Need to lower power for natural convection so water doesn't boil.
    functor = '${fparse 5e4 / 144 * 0.1}'
    variable = T_fluid
  []
[]

[FVInterfaceKernels]
  [water_boundary]
    type = FVConvectionCorrelationInterface
    boundary = 'water_boundary_inwards'
    variable1 = T_solid
    variable2 = T_fluid
    T_fluid = T_fluid
    T_solid = T_solid
    h = 600
    subdomain1 = 'concrete concrete_hd Al'
    subdomain2 = 'water'
    wall_cell_is_bulk = true
  []
[]

[UserObjects]
  [ins_rhie_chow_interpolator]
    type = INSFVRhieChowInterpolator
    pressure = 'pressure'
    u = 'vel_x'
    v = 'vel_y'
    block = 'water'
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  automatic_scaling = true
  off_diagonals_in_auto_scaling = true

  line_search = none
  # Direct solve works for everything small enough
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'

  nl_abs_tol = 1e-8
  nl_max_its = 10
  l_max_its = 3

  steady_state_tolerance = 1e-8
  steady_state_detection = true
  normalize_solution_diff_norm_by_dt = false

  start_time = -1
  dtmax = 43200
  [TimeStepper]
    type = FunctionDT
    function = 'if(t<0.1, 0.1, t)'
  []
[]

[Outputs]
  exodus = true
[]
