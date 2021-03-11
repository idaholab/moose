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
rho_u_in=${fparse u_in * rho_in}
mass_flux_in=${rho_u_in}
eps_in=1
# prescribe outlet pressure = initial pressure on right
p_out=${p_initial_right}
eps_out=0.5
T_in=273.15
two_term_boundary_expansion=true
interp_method='upwind'

[GlobalParams]
  vel = 'velocity'
  # momentum_component = 'nonsense'
[]

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
    two_term_boundary_expansion = ${two_term_boundary_expansion}
  []
  [rho_u]
    type = MooseVariableFVReal
    initial_condition = 1e-15
    two_term_boundary_expansion = ${two_term_boundary_expansion}
  []
  [rho_v]
    type = MooseVariableFVReal
    initial_condition = 1e-15
    two_term_boundary_expansion = ${two_term_boundary_expansion}
  []
  [rho_et]
    type = MooseVariableFVReal
    two_term_boundary_expansion = ${two_term_boundary_expansion}
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
    type = FVMatAdvection
    variable = rho
    advected_quantity = 'superficial_rho'
    advected_interp_method = ${interp_method}
  []

  [momentum_time_x]
    type = FVTimeKernel
    variable = rho_u
  []
  [momentum_advection_x]
    type = FVMatAdvection
    variable = rho_u
    advected_interp_method = ${interp_method}
  []
  [momentum_pressure_x]
    type = PNSFVMomentumPressure
    variable = rho_u
    momentum_component = 'x'
  []

  [momentum_time_y]
    type = FVTimeKernel
    variable = rho_v
  []
  [momentum_advection_y]
    type = FVMatAdvection
    variable = rho_v
    advected_interp_method = ${interp_method}
  []
  [momentum_pressure_y]
    type = PNSFVMomentumPressure
    variable = rho_v
    momentum_component = 'y'
  []

  [energy_time]
    type = FVPorosityTimeDerivative
    variable = rho_et
  []
  [energy_advection]
    type = FVMatAdvection
    variable = rho_et
    advected_quantity = 'superficial_rho_ht'
    advected_interp_method = ${interp_method}
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
    advected_quantity = 'superficial_rho'
    advected_interp_method = ${interp_method}
  []

  [rho_u_left] # implicitly includes advection and pressure
    type = FVDirichletBC
    boundary = 'left'
    variable = rho_u
    value = ${rho_u_in}
  []
  # no advection through walls
  [rho_u_pressure_walls]
    type = PNSFVMomentumPressureBC
    boundary = 'top bottom'
    variable = rho_u
    momentum_component = 'x'
  []
  [rho_u_advection_right]
    type = FVMatAdvectionOutflowBC
    boundary = 'right'
    variable = rho_u
    advected_interp_method = ${interp_method}
  []
  [rho_u_pressure_right] # Normal in x so apply all of pressure
    type = FVNeumannBC
    boundary = 'right'
    variable = rho_u
    value = ${fparse -p_out * eps_out}
  []

  [rho_v_left] # implicitly includes advection and pressure
    type = FVDirichletBC
    boundary = 'left'
    variable = rho_v
    value = 0
  []
  [rho_v_pressure_walls]
    type = PNSFVMomentumPressureBC
    boundary = 'top bottom'
    variable = rho_v
    momentum_component = 'y'
  []
  [rho_v_advection_right]
    type = FVMatAdvectionOutflowBC
    boundary = 'right'
    variable = rho_v
    advected_interp_method = ${interp_method}
  []
  [rho_v_pressure_right] # There is no normal in y so no "pressure flux"
    type = FVNeumannBC
    boundary = 'right'
    variable = rho_v
    value = 0
  []

  [rho_et_left]
    type = PNSFVFluidEnergySpecifiedTemperatureBC
    boundary = 'left'
    variable = rho_et
    temperature = ${T_in}
    superficial_rhou = ${rho_u_in}
    superficial_rhov = 0
    fp = fp
    interp_method = ${interp_method}
  []
  [rho_et_right]
    type = FVMatAdvectionOutflowBC
    boundary = 'right'
    variable = rho_et
    advected_quantity = 'superficial_rho_ht'
    advected_interp_method = ${interp_method}
  []
[]

[Materials]
  [var_mat]
    type = PorousConservedVarMaterial
    rho = rho
    rho_et = rho_et
    superficial_rhou = rho_u
    superficial_rhov = rho_v
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
  nl_abs_tol = 1e-8
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
  petsc_options_value = 'lu       mumps'
  nl_max_its = 10
  steady_state_tolerance = 1e-13
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
