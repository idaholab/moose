# Things we learn from experimenting in 2D
# - mesh refinement needed in the fluid
# - time step size can be ~0.25s (can we improve?)
# - cp multiplier to get to a steady state faster
# - solve heat conduction for steady state directly
# - solver parameters: scaling notably, preconditioner, multi-system!

# Transient parameters
dt_step_multiplier = 2
step_growth_start = 10
cp_multiplier = 1e0
cp_water_multiplier = 1e-3

# Use a correlation for the heat transfer coefficient
h_water = 30

# Fluid properties
mu_multiplier = 1
cp = ${fparse 4181 * cp_water_multiplier}
mu = ${fparse 7.98e-4 * mu_multiplier}

# Mesh discretization
mult = 1
nx = '${fparse 40 * mult}'
ny = '${fparse 32 * mult}'

[Mesh]
  [bulk]
    type = GeneratedMeshGenerator # Can generate simple lines, rectangles and rectangular prisms
    dim = 2
    nx = ${nx}
    ny = ${ny}
    xmax = 10 # m
    ymax = 8 # m
  []

  [create_inner_water]
    type = ParsedSubdomainMeshGenerator
    input = bulk
    combinatorial_geometry = '(x > 2 & x < 3 & y > 1 & y < 5)'
    block_id = 2
  []
  [create_right_water]
    type = ParsedSubdomainMeshGenerator
    input = create_inner_water
    combinatorial_geometry = '(x > 7 & x < 8 & y > 1 & y < 5)'
    block_id = 3
  []

  [hollow_concrete]
    type = ParsedSubdomainMeshGenerator
    input = create_right_water
    block_id = 1
    combinatorial_geometry = 'x > 3.2 & x < 6.8 & y > 1 & y < 5'
  []

  [rename_blocks]
    type = RenameBlockGenerator
    input = hollow_concrete
    old_block = '0 1 2 3'
    new_block = 'concrete cavity water water_inactive'
  []

  [add_water_concrete_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = rename_blocks
    primary_block = water
    paired_block = concrete
    new_boundary = 'water_boundary'
  []
  [add_inactive_water_concrete_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = add_water_concrete_interface
    paired_block = water_inactive
    primary_block = concrete
    new_boundary = 'inactive_water_boundary'
  []

  [add_inner_cavity]
    type = SideSetsBetweenSubdomainsGenerator
    input = add_inactive_water_concrete_interface
    primary_block = concrete
    paired_block = cavity
    new_boundary = 'inner_cavity'
  []

  [add_concrete_outer_boundary]
    type = RenameBoundaryGenerator
    input = add_inner_cavity
    old_boundary = 'left right top bottom'
    new_boundary = 'air_boundary air_boundary air_boundary ground'
  []

  [remove_cavity]
    type = BlockDeletionGenerator
    input = 'add_concrete_outer_boundary'
    block = cavity
  []

  [refine_water]
    type = RefineBlockGenerator
    input = remove_cavity
    refinement = '2'
    block = 'water'
  []
  uniform_refine = 1
  # second_order = true
[]

[Variables]
  [T]
    # Adds a Linear Lagrange variable by default
    block = 'concrete'
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
  # We don't need those kernels to run the water-only problem
  # Solid heat conduction
  [diffusion_concrete]
    type = ADHeatConduction
    variable = T
  []
  # [time_derivative]
  #   type = ADHeatConductionTimeDerivative
  #   variable = T
  # []
[]

[Materials]
  # Materials for the heat conduction equation
  [concrete]
    type = ADHeatConductionMaterial
    block = 'concrete'
    temp = 'T'
    # we keep the high thermal conductivity from the Aluminum
    # in this simplified case to avoid having to mesh an additional layer
    thermal_conductivity_temperature_function = '45'
    specific_heat = '${fparse cp_multiplier * 1170}'
  []
  # This constant is fine as long as there are no mechanical deformations
  [density]
    type = ADGenericConstantMaterial
    block = 'concrete'
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
    # 5MW reactor, 50kW dissipated by radiation, 136 m2 cavity area (in real system)
    # ramp up over 10s
    function = '5e4 / 136'
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
  [solid_to_fluid]
    type = FunctorNeumannBC
    variable = T
    boundary = water_boundary
    functor = 'h_dT'
    coefficient = -1
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

  # Boundary condition functor computing the heat flux with a set heat transfer coefficient
  [heat-flux]
    type = ADParsedFunctorMaterial
    expression = '-${h_water} * (T - T_fluid)'
    functor_names = 'T T_fluid'
    property_name = 'h_dT'
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

  [T_fluid_water_boundary]
    type = FVFunctorNeumannBC
    boundary = water_boundary
    functor = 'h_dT'
    variable = T_fluid
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
  # Nothing defined on water_inactive subdomain
  kernel_coverage_check = false
  material_coverage_check = false
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  line_search = none

  nl_abs_tol = 1e-8

  end_time = 1000
  start_time = -1

  # steady_state_tolerance = 1e-5
  # steady_state_detection = true

  [TimeStepper]
    type = FunctionDT
    function = "if(t<0, 0.1,
                        if(t<${step_growth_start}, 0.25,
                                                   ${fparse 0.25*dt_step_multiplier}))"
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

[Debug]
  show_var_residual_norms = true
[]

[Postprocessors]
  # we will see postprocessors in step 9
  [T_max]
    type = ElementExtremeValue
    variable = T
    value_type = 'max'
    block = 'concrete'
  []
  [T_min]
    type = ElementExtremeValue
    variable = T
    value_type = 'min'
    block = 'concrete'
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
    mu_ave = '${fparse mu_multiplier * 7.98e-4}'
    cp_ave = '${fparse cp_water_multiplier * 4181}'
    k_ave = 0.6
    gravity_magnitude = 9.81
    rho_ave = 955.7
  []
[]
