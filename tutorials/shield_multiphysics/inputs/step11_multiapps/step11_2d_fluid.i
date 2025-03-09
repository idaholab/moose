cp_water_multiplier = 5e-2
mu_multiplier = 1

# Real facility uses forced convection to cool the water tank at full power
# Need to lower power for natural convection so concrete doesn't get too hot.
power = '${fparse 5e4 / 144 * 0.5}'

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'mesh2d_coarse_in.e'
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
    # Cleans up console output
    outputs = none
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
    characteristic_speed = 0.01
  []
  [water_ins_momentum_advection_y]
    type = INSFVMomentumAdvection
    advected_interp_method = upwind
    block = water
    momentum_component = y
    variable = vel_y
    characteristic_speed = 0.1
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

  # Turbulence
  [water_ins_viscosity_rans_x]
    type = INSFVMixingLengthReynoldsStress
    variable = vel_x
    mixing_length = mixing_length
    momentum_component = 'x'
    u = vel_x
    v = vel_y
  []
  [water_ins_viscosity_rans_y]
    type = INSFVMixingLengthReynoldsStress
    variable = vel_y
    mixing_length = mixing_length
    momentum_component = 'y'
    u = vel_x
    v = vel_y
  []
  [water_ins_energy_rans]
    type = WCNSFVMixingLengthEnergyDiffusion
    variable = T_fluid
    cp = cp
    mixing_length = mixing_length
    schmidt_number = 1
    u = vel_x
    v = vel_y
  []
[]

[AuxKernels]
  [mixing_length]
    type = WallDistanceMixingLengthAux
    variable = mixing_length
    walls = 'water_boundary inner_cavity_water'
    execute_on = 'initial'
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
  [total_viscosity]
    type = MixingLengthTurbulentViscosityFunctorMaterial
    u = 'vel_x'
    v = 'vel_y'
    mixing_length = mixing_length
    mu = mu
  []
[]

[FVBCs]
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
    functor = ${power}
    variable = T_fluid
  []
  [T_fluid_water_boundary]
    type = FVFunctorConvectiveHeatFluxBC
    boundary = water_boundary
    variable = T_fluid
    T_bulk = T_fluid
    T_solid = T_solid
    heat_transfer_coefficient = 600
    is_solid = false
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

[AuxVariables]
  # This isn't used in simulation, but useful for visualization
  [vel_z]
    type = INSFVVelocityVariable
    block = 'water'
    initial_condition = 0
  []
  [mixing_length]
    block = 'water'
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
  # This is the variable that is transferred from the main app
  [T_solid]
    block = 'concrete_hd concrete Al'
    initial_condition = 300
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
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu NONZERO superlu_dist'

  nl_abs_tol = 1e-8
  nl_max_its = 10
  l_max_its = 3

  start_time = -1
  dtmax = 100
  [TimeStepper]
    type = FunctionDT
    function = 'if(t < 0.1, 0.1, t)'
  []
[]

[Outputs]
  exodus = true
[]
