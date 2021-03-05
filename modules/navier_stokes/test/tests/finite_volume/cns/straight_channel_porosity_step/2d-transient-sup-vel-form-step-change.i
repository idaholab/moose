advected_interp_method='upwind'
# Establish initial conditions based on STP
rho_initial=1.2754
p_initial=1.01e5
gamma=1.4
e_initial=${fparse p_initial / (gamma - 1) / rho_initial}
# No bulk velocity in the domain initially
et_initial=${e_initial}
rho_et_initial=${fparse rho_initial * et_initial}
# prescribe inlet rho = initial rho
rho_in=${rho_initial}
# u refers to the superficial velocity
u_in=1
mass_flux_in=${fparse u_in * rho_in}
eps_in=1
real_u_in=${fparse u_in / eps_in}
# prescribe inlet pressure = initial pressure
p_in=${p_initial}
momentum_flux_in=${fparse eps_in * p_in}
# prescribe inlet e = initial e
et_in=${fparse e_initial + 0.5 * real_u_in * real_u_in}
# rho_et_in=${fparse rho_in * et_in} # Useful for Dirichlet
ht_in=${fparse et_in + p_in / rho_in}
enthalpy_flux_in=${fparse u_in * rho_in * ht_in}

[Mesh]
  [cartesian]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    nx = 100
    ymin = 0
    ymax = 1
    ny = 10
  []
  [constant_again_porosity]
    input = cartesian
    type = SubdomainBoundingBoxGenerator
    bottom_left = '5 0 0'
    top_right = '10 1 1'
    block_id = 1
  []
[]

[Modules]
  [FluidProperties]
    [fp]
      type = IdealGasFluidProperties
    []
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [rho]
    type = MooseVariableFVReal
  []
  [rho_u]
    type = MooseVariableFVReal
    initial_condition = 1e-15
  []
  [rho_v]
    type = MooseVariableFVReal
    initial_condition = 1e-15
  []
  [rho_et]
    type = MooseVariableFVReal
  []
[]

[ICs]
  [rho_left]
    type = ConstantIC
    value = ${rho_initial}
    block = 0
    variable = rho
  []
  [rho_right]
    type = ConstantIC
    value = ${fparse 2 * rho_initial}
    block = 1
    variable = rho
  []
  [rho_et_left]
    type = ConstantIC
    value = ${rho_et_initial}
    variable = rho_et
    block = 0
  []
  [rho_et_right]
    type = ConstantIC
    value = ${fparse 2 * rho_et_initial}
    variable = rho_et
    block = 1
  []
[]

[AuxVariables]
  [specific_volume]
    type = MooseVariableFVReal
  []
  [vel_x]
    type = MooseVariableFVReal
  []
  [porosity]
    type = MooseVariableFVReal
  []
  [real_vel_x]
    type = MooseVariableFVReal
  []
  [specific_internal_energy]
    type = MooseVariableFVReal
  []
  [pressure]
    type = MooseVariableFVReal
  []
  [mach]
    type = MooseVariableFVReal
  []
  [mass_flux]
    type = MooseVariableFVReal
  []
  [momentum_flux]
    type = MooseVariableFVReal
  []
  [enthalpy_flux]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [specific_volume]
    type = SpecificVolumeAux
    variable = specific_volume
    rho = rho
    execute_on = 'timestep_end'
  []
  [vel_x]
    type = NSVelocityAux
    variable = vel_x
    rho = rho
    momentum = rho_u
    execute_on = 'timestep_end'
  []
  [porosity]
    type = MaterialRealAux
    variable = porosity
    property = porosity
    execute_on = 'timestep_end'
  []
  [real_vel_x]
    type = ParsedAux
    variable = real_vel_x
    function = 'vel_x / porosity'
    args = 'vel_x porosity'
    execute_on = 'timestep_end'
  []
  [specific_internal_energy]
    type = ParsedAux
    variable = specific_internal_energy
    function = 'rho_et / rho - (real_vel_x * real_vel_x) / 2'
    args = 'rho_et rho real_vel_x'
    execute_on = 'timestep_end'
  []
  [pressure]
    type = NSPressureAux
    variable = pressure
    specific_volume = specific_volume
    e = specific_internal_energy
    fluid_properties = fp
    execute_on = 'timestep_end'
  []
  # [mach]
  #   type = NSMachAux
  #   variable = mach
  #   vel_x = real_vel_x
  #   e = specific_internal_energy
  #   specific_volume = specific_volume
  #   execute_on = 'timestep_end'
  #   fluid_properties = 'fp'
  # []
  [mass_flux]
    type = ParsedAux
    variable = mass_flux
    function = 'rho_u'
    args = 'rho_u'
    execute_on = 'timestep_end'
  []
  [momentum_flux]
    type = ParsedAux
    variable = momentum_flux
    function = 'vel_x * rho_u / porosity + pressure * porosity'
    args = 'vel_x rho_u porosity pressure'
    execute_on = 'timestep_end'
  []
  [enthalpy_flux]
    type = ParsedAux
    variable = enthalpy_flux
    function = 'vel_x * (rho_et + pressure)'
    args = 'vel_x rho_et pressure'
    execute_on = 'timestep_end'
  []
[]

[FVKernels]
  [mass_time]
    type = FVPorosityTimeDerivative
    variable = rho
  []
  [mass_advection]
    type = PNSFVUpwindAdvectorAndAdvected
    variable = rho
    vel = superficial_velocity
    advected_interp_method = ${advected_interp_method}
  []

  [rho_u_pressure]
    type = PNSFVMomentumPressure
    variable = rho_u
    momentum_component = 'x'
  []

  [rho_v_time]
    type = FVTimeKernel
    variable = rho_v
  []
  [rho_v_advection_and_pressure]
    type = PNSFVMomentumOneOverGammaUpwindAdvectorAndAdvected
    variable = rho_v
    vel = superficial_velocity
  []
  [rho_v_pressure]
    type = PNSFVMomentumPressure
    variable = rho_v
    momentum_component = 'y'
  []

  [energy_time]
    type = FVPorosityTimeDerivative
    variable = rho_et
  []
  [energy_advection]
    type = PNSFVUpwindAdvectorAndAdvected
    variable = rho_et
    advected_quantity = 'rho_ht'
    vel = superficial_velocity
    advected_interp_method = ${advected_interp_method}
  []
[]

[FVBCs]
  [rho_left]
    type = FVNeumannBC
    boundary = 'left'
    variable = rho
    value = ${mass_flux_in}
  []
  [rho_right]
    type = FVMatAdvectionOutflowBC
    boundary = 'right'
    variable = rho
    vel = superficial_velocity
    advected_interp_method = ${advected_interp_method}
  []
  [rho_u_left]
    type = FVNeumannBC
    boundary = 'left'
    variable = rho_u
    value = ${momentum_flux_in}
  []
  [rho_u_right]
    type = PNSFVMomentumPressureBC
    variable = rho_u
    boundary = 'right'
    momentum_component = 'x'
  []
  [rho_v_pressure]
    type = PNSFVMomentumPressureBC
    boundary = 'top bottom left right'
    variable = rho_v
    momentum_component = 'y'
  []
  [rho_v_advection_outlet]
    type = PNSFVMomentumOneOverGammaMatAdvectionOutflowBC
    boundary = 'right'
    variable = rho_v
    vel = superficial_velocity
  []
  [rho_et_left]
    type = FVNeumannBC
    boundary = 'left'
    variable = rho_et
    value = ${enthalpy_flux_in}
  []
  [rho_et_right]
    type = FVMatAdvectionOutflowBC
    boundary = 'right'
    variable = rho_et
    vel = superficial_velocity
    advected_quantity = 'rho_ht'
    advected_interp_method = ${advected_interp_method}
  []
[]

[Materials]
  [var_mat]
    type = PorousConservedVarMaterial
    rho = rho
    superficial_rhou = rho_u
    superficial_rhov = rho_v
    rho_et = rho_et
    fp = fp
    porosity = porosity
  []
  [porosity_left]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '1'
    block = 0
  []
  [porosity_right]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '0.5'
    block = 1
  []
[]

[Functions]
  [changing_eps]
    type = ParsedFunction
    value = '-1/20 * x + 3/2'
  []
[]

[Executioner]
  solve_type = NEWTON
  nl_rel_tol = 1e-8
  nl_abs_tol = 2e-6
  type = Transient
  num_steps = 1000
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-8
    optimal_iterations = 6
  []
  steady_state_detection = true
  line_search = 'bt'
  abort_on_solve_fail = true
  verbose = true
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       superlu_dist'
  nl_max_its = 10
  ss_check_tol = 1e-12
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'initial timestep_end'
  []
[]

[Debug]
  show_var_residual_norms = true
[]
