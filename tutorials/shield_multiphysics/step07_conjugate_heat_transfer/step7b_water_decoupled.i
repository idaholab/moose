cp_water_multiplier = 1e-4
mu_multiplier = 1

# Notes:
# While it solves, it's clearly not refined enough to show the true behavior
# See step 7c

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = '../step03_boundary_conditions/mesh_in.e'
  []
  [refine_water]
    type = RefineBlockGenerator
    input = fmg
    refinement = '1'
    block = 'water'
  []
[]

[AuxVariables]
  [T]
    block = 'concrete_hd concrete Al'
    initial_condition = 370
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
  [vel_z]
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
    point = '2 9 2'
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
  [water_ins_momentum_time_vel_z]
    type = INSFVMomentumTimeDerivative
    block = water
    momentum_component = z
    variable = vel_z
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
  [water_ins_momentum_advection_z]
    type = INSFVMomentumAdvection
    advected_interp_method = upwind
    block = water
    momentum_component = z
    variable = vel_z
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
  [water_ins_momentum_diffusion_z]
    type = INSFVMomentumDiffusion
    block = water
    momentum_component = z
    mu = mu
    variable = vel_z
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
  [water_ins_momentum_pressure_z]
    type = INSFVMomentumPressure
    block = water
    momentum_component = z
    pressure = pressure
    variable = vel_z
  []
  [water_ins_momentum_gravity_z]
    type = INSFVMomentumGravity
    block = water
    gravity = '0 0 -9.81'
    momentum_component = z
    variable = vel_z
  []
  [water_ins_momentum_boussinesq_z]
    type = INSFVMomentumBoussinesq
    T_fluid = T_fluid
    alpha_name = alpha
    block = water
    gravity = '0 0 -9.81'
    momentum_component = z
    ref_temperature = 300
    rho = 955.7
    variable = vel_z
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
    boundary = water_boundary
    function = 0
    variable = vel_x
  []
  [vel_y_water_boundary]
    type = INSFVNoSlipWallBC
    boundary = water_boundary
    function = 0
    variable = vel_y
  []
  [vel_z_water_boundary]
    type = INSFVNoSlipWallBC
    boundary = water_boundary
    function = 0
    variable = vel_z
  []

  [T_fluid_water_boundary]
    type = FVFunctorNeumannBC
    boundary = water_boundary
    functor = ${fparse 5e4 / 136}
    variable = T_fluid
  []
[]

[UserObjects]
  [ins_rhie_chow_interpolator]
    type = INSFVRhieChowInterpolator
    pressure = 'pressure'
    u = 'vel_x'
    v = 'vel_y'
    w = 'vel_z'
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

  end_time = 100
  dt = 0.25
  start_time = -2

  # steady_state_tolerance = 1e-5
  # steady_state_detection = true

  [TimeStepper]
    type = FunctionDT
    function = 'if(t<0,0.1,1)'
  []
[]

[Outputs]
  exodus = true
[]
