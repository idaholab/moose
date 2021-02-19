advected_interp_method='upwind'
rho_stp=1.2754
p_stp=1.01e5
gamma=1.4
e_stp=${fparse p_stp / (gamma - 1) / rho_stp}
et_stp=${e_stp}
rho_et_stp=${fparse rho_stp * et_stp}
rho_in=${rho_stp}
u_in=1
eps_in=1
eps_u_rho_in=${fparse rho_in * eps_in * u_in}
eps_u_rho_u_in=${fparse rho_in * eps_in * u_in * u_in}
et_in=${fparse e_stp + 0.5 * u_in * u_in}
p_in=${p_stp}
ht_in=${fparse et_in + p_in / rho_in}
eps_u_rho_ht_in=${fparse rho_in * eps_in * u_in * ht_in}

[Mesh]
  [cartesian]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 30
    nx = 30000
  []
  [changing_porosity]
    input = cartesian
    type = SubdomainBoundingBoxGenerator
    bottom_left = '10 0 0'
    top_right = '20 1 1'
    block_id = 1
  []
  [constant_again_porosity]
    input = changing_porosity
    type = SubdomainBoundingBoxGenerator
    bottom_left = '20 0 0'
    top_right = '30 1 1'
    block_id = 2
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
    initial_condition = ${rho_stp}
  []
  [rho_u]
    type = MooseVariableFVReal
    initial_condition = 1e-15
  []
  [rho_et]
    type = MooseVariableFVReal
    initial_condition = ${rho_et_stp}
  []
[]

[AuxVariables]
  [specific_volume]
    type = MooseVariableFVReal
  []
  [pressure]
    type = MooseVariableFVReal
  []
  [specific_internal_energy]
    type = MooseVariableFVReal
  []
  [vel_x]
    type = MooseVariableFVReal
  []
  [porosity]
    type = MooseVariableFVReal
  []
  [superficial_vel_x]
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
  [specific_internal_energy]
    type = NSInternalEnergyAux
    variable = specific_internal_energy
    rho = rho
    vel_x = vel_x
    rho_et = rho_et
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
  [porosity]
    type = MaterialRealAux
    variable = porosity
    property = porosity
    execute_on = 'timestep_end'
  []
  [superficial_vel_x]
    type = ParsedAux
    variable = superficial_vel_x
    function = 'vel_x * porosity'
    args = 'vel_x porosity'
    execute_on = 'timestep_end'
  []
  [mach]
    type = NSMachAux
    variable = mach
    vel_x = vel_x
    e = specific_internal_energy
    specific_volume = specific_volume
    execute_on = 'timestep_end'
    fluid_properties = 'fp'
  []
  [mass_flux]
    type = ParsedAux
    variable = mass_flux
    function = 'superficial_vel_x * rho'
    args = 'rho superficial_vel_x'
    execute_on = 'timestep_end'
  []
  [momentum_flux]
    type = ParsedAux
    variable = momentum_flux
    function = 'superficial_vel_x * rho_u'
    args = 'superficial_vel_x rho_u'
    execute_on = 'timestep_end'
  []
  [enthalpy_flux]
    type = ParsedAux
    variable = enthalpy_flux
    function = 'superficial_vel_x * rho * (specific_internal_energy + 0.5 * vel_x * vel_x + pressure / rho)'
    args = 'superficial_vel_x rho specific_internal_energy vel_x pressure'
    execute_on = 'timestep_end'
  []
[]

[FVKernels]
  [mass_time]
    type = FVPorosityTimeDerivative
    variable = rho
  []
  [mass_advection]
    type = FVMatAdvection
    variable = rho
    vel = superficial_velocity
    advected_interp_method = ${advected_interp_method}
  []

  [momentum_time]
    type = FVPorosityTimeDerivative
    variable = rho_u
  []
  [momentum_advection]
    type = FVMatAdvection
    variable = rho_u
    vel = superficial_velocity
    advected_interp_method = ${advected_interp_method}
  []
  [momentum_pressure]
    type = FVPorosityMomentumPressure
    variable = rho_u
    momentum_component = 'x'
  []

  [energy_time]
    type = FVPorosityTimeDerivative
    variable = rho_et
  []
  [energy_advection]
    type = FVMatAdvection
    variable = rho_et
    advected_quantity = 'rho_ht'
    vel = superficial_velocity
    advected_interp_method = ${advected_interp_method}
  []
[]

[FVBCs]
  # [rho_left]
  #   type = FVDirichletBC
  #   boundary = 'left'
  #   variable = rho
  #   value = 1
  # []
  [rho_left]
    type = FVNeumannBC
    boundary = 'left'
    variable = rho
    value = ${eps_u_rho_in}
  []
  [rho_right]
    type = FVMatAdvectionOutflowBC
    boundary = 'right'
    variable = rho
    vel = superficial_velocity
    advected_interp_method = ${advected_interp_method}
  []
  # [rho_u_left]
  #   type = FVDirichletBC
  #   boundary = 'left'
  #   variable = rho_u
  #   value = 1
  # []
  [rho_u_left]
    type = FVNeumannBC
    boundary = 'left'
    variable = rho_u
    value = ${eps_u_rho_u_in}
  []
  [rho_u_right]
    type = FVMatAdvectionOutflowBC
    boundary = 'right'
    variable = rho_u
    vel = superficial_velocity
    advected_interp_method = ${advected_interp_method}
  []
  # [rho_et_left]
  #   type = FVDirichletBC
  #   boundary = 'left'
  #   variable = rho_et
  #   value = 10.5
  # []
  [rho_et_left]
    type = FVNeumannBC
    boundary = 'left'
    variable = rho_et
    value = ${eps_u_rho_ht_in}
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
    type = ConservedVarMaterial
    rho = rho
    rhou = rho_u
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
  [porosity_changing]
    type = GenericFunctionMaterial
    prop_names = 'porosity'
    prop_values = 'changing_eps'
    block = 1
  []
  [porosity_right]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '0.5'
    block = 2
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
  nl_abs_tol = 1e-7
  type = Transient
  num_steps = 1000
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-4
    optimal_iterations = 6
  []
  steady_state_detection = true
  line_search = 'none'
  automatic_scaling = true
  compute_scaling_once = false
  verbose = true
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       strumpack'
[]

[Outputs]
  exodus = true
  checkpoint = true
[]

[Debug]
  show_var_residual_norms = true
[]
