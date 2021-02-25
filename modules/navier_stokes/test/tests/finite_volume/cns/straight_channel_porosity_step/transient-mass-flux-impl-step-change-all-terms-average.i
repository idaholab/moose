# upwind is not stable
flux_interp_method='average'
# Establish initial conditions based on STP
rho_initial_left=1.2754
p_initial_left=1.01e5
p_initial_right=${fparse 2 * p_initial_left}
gamma=1.4
e_initial_left=${fparse p_initial_left / (gamma - 1) / rho_initial_left}
# No bulk velocity in the domain initially
et_initial_left=${e_initial_left}
rho_et_initial_left=${fparse rho_initial_left * et_initial_left}
# prescribe inlet rho = initial rho
rho_in=${rho_initial_left}
# u refers to the superficial velocity
u_in=1
mass_flux_in=${fparse u_in * rho_in}
eps_in=1
real_u_in=${fparse u_in / eps_in}
advective_momentum_flux_in=${fparse rho_in * u_in * real_u_in}
# prescribe outlet pressure = initial pressure on right
p_out=${p_initial_right}
eps_out=0.5
# prescribe inlet e = initial e on left
e_in = ${e_initial_left}
ht_in=${fparse 0.5 * real_u_in * real_u_in + gamma * e_in}
enthalpy_flux_in=${fparse u_in * rho_in * ht_in}

# [GlobalParams]
#   vel = 'nonsense'
#   advected_interp_method = 'nonsense'
#   momentum_component = 'nonsense'
#   advected_quantity = 'nonsense'
# []

[Mesh]
  [cartesian]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 10
    nx = 1000
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
  [rho_et]
    type = MooseVariableFVReal
  []
[]

[ICs]
  [rho_left]
    type = ConstantIC
    value = ${rho_initial_left}
    block = 0
    variable = rho
  []
  [rho_right]
    type = ConstantIC
    value = ${fparse 2 * rho_initial_left}
    block = 1
    variable = rho
  []
  [rho_et_left]
    type = ConstantIC
    value = ${rho_et_initial_left}
    variable = rho_et
    block = 0
  []
  [rho_et_right]
    type = ConstantIC
    value = ${fparse 2 * rho_et_initial_left}
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
  [mach]
    type = NSMachAux
    variable = mach
    vel_x = real_vel_x
    e = specific_internal_energy
    specific_volume = specific_volume
    execute_on = 'timestep_end'
    fluid_properties = 'fp'
  []
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
    type = NSFVAdvection
    variable = rho
    flux_interp_method = ${flux_interp_method}
    advected_quantity = 1
  []

  [momentum_time]
    type = FVTimeKernel
    variable = rho_u
  []
  [momentum_advection]
    type = NSFVAdvection
    variable = rho_u
    flux_interp_method = ${flux_interp_method}
    advected_quantity = 'vel_x'
  []
  [momentum_pressure]
    type = NSFVPorosityMomentumPressure
    variable = rho_u
    momentum_component = 'x'
  []

  [energy_time]
    type = FVPorosityTimeDerivative
    variable = rho_et
  []
  [energy_advection]
    type = NSFVAdvection
    variable = rho_et
    advected_quantity = 'ht'
    flux_interp_method = ${flux_interp_method}
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
    type = NSFVAdvectionFluxBC
    boundary = 'right'
    variable = rho
    advected_quantity = 1
    flux_interp_method = ${flux_interp_method}
  []
  # [rho_u_advection]
  #   type = NSFVAdvectionFluxBC
  #   boundary = 'left right'
  #   variable = rho_u
  #   advected_quantity = 'vel_x'
  #   flux_interp_method = ${flux_interp_method}
  # []
  [rho_u_advection_left]
    type = FVNeumannBC
    boundary = 'left'
    variable = rho_u
    value = ${advective_momentum_flux_in}
  []
  [rho_u_advection_right]
    type = NSFVAdvectionFluxBC
    boundary = 'right'
    variable = rho_u
    advected_quantity = 'vel_x'
    flux_interp_method = ${flux_interp_method}
  []
  [rho_u_pressure_left]
    type = NSFVPorosityMomentumPressureBC
    boundary = 'left'
    variable = rho_u
    momentum_component = 'x'
  []
  [rho_u_pressure_right]
    type = FVNeumannBC
    boundary = 'right'
    variable = rho_u
    value = ${fparse -p_out * eps_out}
  []
  [rho_et_left]
    type = FVNeumannBC
    boundary = 'left'
    variable = rho_et
    value = ${enthalpy_flux_in}
  []
  [rho_et_right]
    type = NSFVAdvectionFluxBC
    boundary = 'right'
    variable = rho_et
    advected_quantity = 'ht'
    flux_interp_method = ${flux_interp_method}
  []
[]

[Materials]
  [var_mat]
    type = PorousConservedVarMaterial
    rho = rho
    rho_et = rho_et
    superficial_rhou = rho_u
    fp = fp
    porosity = porosity
  []
  [porosity_left]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '${eps_in}'
    block = 0
  []
  [porosity_right]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '${eps_out}'
    block = 1
  []
[]

[Executioner]
  solve_type = NEWTON
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-9
  type = Transient
  num_steps = 1000
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-9
    optimal_iterations = 6
  []
  steady_state_detection = true
  line_search = 'bt'
  abort_on_solve_fail = true
  verbose = true
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       mumps'
  nl_max_its = 10
  ss_check_tol = 1e-13
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

[Postprocessors]
  [mass_flux_left]
    type = SideIntegralVariablePostprocessor
    variable = rho_u
    boundary = 'left'
  []
  [mass_flux_right]
    type = SideIntegralVariablePostprocessor
    variable = rho_u
    boundary = 'right'
  []
[]
