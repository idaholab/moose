# Things we learn from experimenting in 2D
# - cp multiplier to get to a steady state faster
# - solve heat conduction for steady state directly
# - solver parameters: scaling notably, preconditioner, multi-system!

# Fluid properties
cp_water_multiplier = 1e-4
mu_multiplier = 1e4
cp = ${fparse 4181 * cp_water_multiplier}
mu = ${fparse 7.98e-4 * mu_multiplier}

# Power
# Real facility uses forced convection to cool the water tank at full power
# Need to lower power for natural convection so water doesn't boil.
power = ${fparse 5e4 / 144 * 0.5}

# Coupling
htc = 600

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
    solver_sys = 'heat'
  []

  # Fluid flow variables
  [vel_x]
    type = INSFVVelocityVariable
    block = 'water'
    initial_condition = 1e-4
    solver_sys = 'flow'
  []
  [vel_y]
    type = INSFVVelocityVariable
    block = 'water'
    initial_condition = 1e-4
    solver_sys = 'flow'
  []
  [pressure]
    type = INSFVPressureVariable
    block = 'water'
    initial_condition = 1e5
    solver_sys = 'flow'
  []
  [T_fluid]
    type = INSFVEnergyVariable
    initial_condition = 300
    block = 'water'
    scaling = 1e-3
    solver_sys = 'flow'
  []
  [lambda]
    type = MooseVariableScalar
    family = SCALAR
    order = FIRST
    solver_sys = 'flow'
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
  # Materials for the heat conduction equation
  [concrete_hd]
    type = ADHeatConductionMaterial
    block = 'concrete_hd'
    temp = 'T'
    # we specify a function of time, temperature is passed as the time argument
    # in the material
    thermal_conductivity_temperature_function = '5.0 + 0.001 * t'
  []
  [concrete]
    type = ADHeatConductionMaterial
    block = 'concrete'
    temp = 'T'
    thermal_conductivity_temperature_function = '2.25 + 0.001 * t'
  []
  [Al]
    type = ADHeatConductionMaterial
    block = 'Al'
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
    value = '${power}'
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

  # Heat fluxes between water and concrete
  [fluid_to_solid]
    type = ADConvectiveHeatFluxBC
    boundary = water_boundary_inwards
    variable = T
    T_infinity_functor = T_fluid_interface_avg
    heat_transfer_coefficient_functor = ${htc}
  []
[]

[GlobalParams]
  # Common numerical parameters in INSFV
  velocity_interp_method = rc
  advected_interp_method = upwind
  rhie_chow_user_object = ins_rhie_chow_interpolator
  rho = rho
[]

[FVKernels]
  # Mass conservation equation
  [water_ins_mass_advection]
    type = INSFVMassAdvection
    variable = pressure
  []

  # Pressure constraint
  [water_ins_mass_pressure_pin]
    type = FVIntegralValueConstraint
    lambda = lambda
    phi0 = 1e5
    variable = pressure
  []

  # Momentum conservation equations
  [water_ins_momentum_time_vel_x]
    type = INSFVMomentumTimeDerivative
    momentum_component = x
    variable = vel_x
  []
  [water_ins_momentum_time_vel_y]
    type = INSFVMomentumTimeDerivative
    momentum_component = y
    variable = vel_y
  []
  [water_ins_momentum_advection_x]
    type = INSFVMomentumAdvection
    momentum_component = x
    variable = vel_x
  []
  [water_ins_momentum_advection_y]
    type = INSFVMomentumAdvection
    momentum_component = y
    variable = vel_y
  []
  [water_ins_momentum_diffusion_x]
    type = INSFVMomentumDiffusion
    momentum_component = x
    mu = mu
    variable = vel_x
  []
  [water_ins_momentum_diffusion_y]
    type = INSFVMomentumDiffusion
    momentum_component = y
    mu = mu
    variable = vel_y
  []
  [water_ins_momentum_pressure_x]
    type = INSFVMomentumPressure
    momentum_component = x
    pressure = pressure
    variable = vel_x
  []
  [water_ins_momentum_pressure_y]
    type = INSFVMomentumPressure
    momentum_component = y
    pressure = pressure
    variable = vel_y
  []
  [water_ins_momentum_gravity_z]
    type = INSFVMomentumGravity
    gravity = '0 -9.81 0'
    momentum_component = y
    variable = vel_y
  []
  [water_ins_momentum_boussinesq_z]
    type = INSFVMomentumBoussinesq
    T_fluid = T_fluid
    alpha_name = alpha
    gravity = '0 -9.81 0'
    momentum_component = y
    ref_temperature = 300
    rho = 955.7
    variable = vel_y
  []

  # Energy conservation equation
  [water_ins_energy_time]
    type = INSFVEnergyTimeDerivative
    dh_dt = dh_dt
    rho = rho
    variable = T_fluid
  []
  [water_ins_energy_advection]
    type = INSFVEnergyAdvection
    variable = T_fluid
  []
  [water_ins_energy_diffusion_all]
    type = FVDiffusion
    coeff = k
    variable = T_fluid
  []
[]

[FunctorMaterials]
  [water]
    type = ADGenericFunctorMaterial
    block = 'water'
    prop_names = 'rho    k     cp  mu    alpha_wall alpha'
    prop_values = '955.7 0.6 ${cp} ${mu} 30         2.9e-3'
  []
  [water_ins_enthalpy_material]
    type = INSFVEnthalpyFunctorMaterial
    block = water
    cp = cp
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

  [T_fluid_inner_cavity]
    type = FVFunctorNeumannBC
    boundary = inner_cavity_water
    # Real facility uses forced convection to cool the water tank at full power
    # Need to lower power for natural convection so water doesn't boil.
    functor = ${power}
    variable = T_fluid
  []
  [T_fluid_water_boundary]
    type = FVFunctorConvectiveHeatFluxBC
    boundary = water_boundary
    variable = T_fluid
    T_bulk = T_fluid
    T_solid = T_solid_interface_avg
    heat_transfer_coefficient = ${htc}
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
  nl_sys_names = 'heat flow'
  kernel_coverage_check = false
  material_coverage_check = false
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  line_search = none

  nl_abs_tol = 1e-8
  nl_max_its = 15

  steady_state_tolerance = 1e-6
  steady_state_detection = true

  start_time = -1
  [TimeStepper]
    type = FunctionDT
    function = 'if(t<0.1, 0.1, t)'
  []
[]

[Preconditioning]
  [thermomecha]
    type = SMP
    nl_sys = 'heat'
    petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
    petsc_options_value = 'hypre boomeramg 500'
  []
  [flow]
    type = SMP
    nl_sys = 'flow'
    petsc_options_iname = '-pc_type -pc_factor_shift_type'
    petsc_options_value = 'lu NONZERO'
  []
[]

[Outputs]
  exodus = true
[]

# we will see postprocessors in step 9
[Postprocessors]
  # Postprocessors used for coupling
  [T_solid_interface_avg]
    type = SideAverageValue
    boundary = water_boundary_inwards
    variable = T
    # Want to compute initial value and after each timestep for coupling
    execute_on = 'initial timestep_end'
  []
  [T_fluid_interface_avg]
    type = SideAverageValue
    boundary = water_boundary
    variable = T_fluid
    # Want to compute initial value and after each timestep for coupling
    execute_on = 'initial timestep_end'
  []

  # Useful information
  [T_max]
    type = ElementExtremeValue
    variable = T
    value_type = 'max'
    block = 'concrete_hd concrete'
  []
  [T_min]
    type = ElementExtremeValue
    variable = T
    value_type = 'min'
    block = 'concrete_hd concrete'
  []
  [T_water_max]
    type = ElementExtremeValue
    variable = T_fluid
    value_type = 'max'
    block = 'water'
  []
  [T_water_min]
    type = ElementExtremeValue
    variable = T_fluid
    value_type = 'min'
    block = 'water'
  []
  [Ra]
    type = RayleighNumber
    beta = 2.9e-3
    T_hot = T_max
    T_cold = T_min
    l = 4
    mu_ave = '${mu}'
    cp_ave = '${cp}'
    k_ave = 0.6
    gravity_magnitude = 9.81
    rho_ave = 955.7
  []
[]
