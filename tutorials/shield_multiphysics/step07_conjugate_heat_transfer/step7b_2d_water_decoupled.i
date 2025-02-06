cp_water_multiplier = 1e-4
mu_multiplier = 1e4

# Notes:
# While it solves, it's clearly not refined enough to show the true behavior
# See step 7c

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'mesh2d_in.e'
  []
  [remove_solid]
    type = BlockDeletionGenerator
    input = fmg
    block = 'concrete_hd concrete Al'
  []
[]

[Variables]
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
  [vel_x_water_boundary]
    type = INSFVNoSlipWallBC
    boundary = 'water_boundary water_bottom inner_cavity_water'
    function = 0
    variable = vel_x
  []
  [vel_y_water_boundary]
    type = INSFVNoSlipWallBC
    boundary = 'water_boundary water_bottom inner_cavity_water'
    function = 0
    variable = vel_y
  []

  [T_fluid_inner_cavity]
    type = FVFunctorNeumannBC
    boundary = inner_cavity_water
    # Real facility uses forced convection to cool the water tank at full power
    # Need to lower power for natural convection so water doesn't boil.
    functor = ${fparse 5e4 / 144 * 0.06}
    variable = T_fluid
  []
  [T_fluid_water_boundary]
    type = FVFunctorConvectiveHeatFluxBC
    boundary = water_boundary
    variable = T_fluid
    T_bulk = T_fluid
    T_solid = 300
    heat_transfer_coefficient = 600
    is_solid = true
  []
  [T_fluid_bottom]
    type = FVDirichletBC
    boundary = water_bottom
    variable = T_fluid
    value = 300
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

  steady_state_tolerance = 1e-6
  steady_state_detection = true

  start_time = -1
  [TimeStepper]
    type = FunctionDT
    function = 'if(t<0.1, 0.1, t)'
  []
[]

[Outputs]
  exodus = true
[]
