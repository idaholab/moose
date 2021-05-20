two_term_boundary_expansion=true
rho0 = 3.487882614709243
u0 = 1
eps = 0.9
rho_ud0 = ${fparse rho0*u0*eps}
rho_et0 = 26.74394130735463
interp_method='upwind'

[GlobalParams]
  vel = 'velocity'
  # momentum_component = 'nonsense'
[]

[Mesh]
  [cartesian]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = .1
    xmax = 1.1
    nx = 2
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
    initial_condition = ${rho0}
  []
  [rho_ud]
    type = MooseVariableFVReal
    initial_condition = ${rho_ud0}
    two_term_boundary_expansion = ${two_term_boundary_expansion}
  []
  [rho_et]
    type = MooseVariableFVReal
    two_term_boundary_expansion = ${two_term_boundary_expansion}
    initial_condition = ${rho_et0}
  []
[]

[AuxVariables]
  [specific_volume]
    type = MooseVariableFVReal
  []
  [superficial_vel_x]
    type = MooseVariableFVReal
  []
  [porosity]
    type = MooseVariableFVReal
  []
  [vel_x]
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
  [superficial_vel_x]
    type = NSVelocityAux
    variable = superficial_vel_x
    rho = rho
    momentum = rho_ud
    execute_on = 'timestep_end'
  []
  [porosity]
    type = MaterialRealAux
    variable = porosity
    property = porosity
    execute_on = 'timestep_end'
  []
  [vel_x]
    type = ParsedAux
    variable = vel_x
    function = 'superficial_vel_x / porosity'
    args = 'superficial_vel_x porosity'
    execute_on = 'timestep_end'
  []
  [specific_internal_energy]
    type = ParsedAux
    variable = specific_internal_energy
    function = 'rho_et / rho - (vel_x * vel_x) / 2'
    args = 'rho_et rho vel_x'
    execute_on = 'timestep_end'
  []
  [pressure]
    type = ADMaterialRealAux
    variable = pressure
    property = pressure
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
    function = 'rho_ud'
    args = 'rho_ud'
    execute_on = 'timestep_end'
  []
  [momentum_flux]
    type = ParsedAux
    variable = momentum_flux
    function = 'superficial_vel_x * rho_ud / porosity + pressure * porosity'
    args = 'superficial_vel_x rho_ud porosity pressure'
    execute_on = 'timestep_end'
  []
  [enthalpy_flux]
    type = ParsedAux
    variable = enthalpy_flux
    function = 'superficial_vel_x * (rho_et + pressure)'
    args = 'superficial_vel_x rho_et pressure'
    execute_on = 'timestep_end'
  []
[]

[FVKernels]
  [mass_advection]
    type = FVMatAdvection
    variable = rho
    advected_quantity = 'superficial_rho'
    advected_interp_method = ${interp_method}
  []
  [mass_fn]
    type = FVBodyForce
    variable = rho
    function = 'forcing_rho'
  []

  [momentum_advection_x]
    type = FVMatAdvection
    variable = rho_ud
    advected_interp_method = ${interp_method}
  []
  [momentum_pressure_x]
    type = PCNSFVMomentumPressureFlux
    variable = rho_ud
    momentum_component = 'x'
  []
  [momentum_fn]
    type = FVBodyForce
    variable = rho_ud
    function = 'forcing_rho_ud'
  []

  [energy_advection]
    type = FVMatAdvection
    variable = rho_et
    advected_quantity = 'superficial_rho_ht'
    advected_interp_method = ${interp_method}
  []
  [energy_fn]
    type = FVBodyForce
    variable = rho_et
    function = 'forcing_rho_et'
  []
[]

[FVBCs]
  [rho_left]
    type = FVFunctionNeumannBC
    boundary = 'left'
    variable = rho
    function = exact_rho_ud
  []
  [rho_right]
    type = FVMatAdvectionOutflowBC
    boundary = 'right'
    variable = rho
    advected_quantity = 'superficial_rho'
    advected_interp_method = ${interp_method}
  []

  [rho_ud_left] # implicitly includes advection and pressure
    type = FVFunctionDirichletBC
    boundary = 'left'
    variable = rho_ud
    function = exact_rho_ud
  []
  [rho_ud_advection_right]
    type = FVMatAdvectionOutflowBC
    boundary = 'right'
    variable = rho_ud
    advected_interp_method = ${interp_method}
  []
  [rho_ud_pressure_right] # Normal in x so apply all of pressure
    type = FVFunctionNeumannBC
    boundary = 'right'
    variable = rho_ud
    function = 'exact_eps_p'
    factor = -1
  []

  [rho_et_left]
    type = PNSFVFluidEnergySpecifiedTemperatureBC
    boundary = 'left'
    variable = rho_et
    temperature = exact_T
    superficial_rhou = exact_rho_ud
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
    superficial_rhou = rho_ud
    fp = fp
    porosity = porosity
  []
  [porosity]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '${eps}'
  []
[]

[Functions]
[exact_rho]
  type = ParsedFunction
  value = '3.48788261470924*cos(x)'
[]
[forcing_rho]
  type = ParsedFunction
  value = '-3.83667087618017*eps*sin(1.1*x)'
  vars = 'eps'
  vals = '${eps}'
[]
[exact_rho_ud]
  type = ParsedFunction
  value = '3.48788261470924*eps*cos(1.1*x)'
  vars = 'eps'
  vals = '${eps}'
[]
[forcing_rho_ud]
  type = ParsedFunction
  value = 'eps*(-(10.6975765229419*cos(1.2*x)/cos(x) - 0.697576522941849*cos(1.1*x)^2/cos(x)^2)*sin(x) + (10.6975765229419*sin(x)*cos(1.2*x)/cos(x)^2 - 1.3951530458837*sin(x)*cos(1.1*x)^2/cos(x)^3 + 1.53466835047207*sin(1.1*x)*cos(1.1*x)/cos(x)^2 - 12.8370918275302*sin(1.2*x)/cos(x))*cos(x)) + 3.48788261470924*eps*sin(x)*cos(1.1*x)^2/cos(x)^2 - 7.67334175236034*eps*sin(1.1*x)*cos(1.1*x)/cos(x)'
  vars = 'eps'
  vals = '${eps}'
[]
[exact_rho_et]
  type = ParsedFunction
  value = '26.7439413073546*cos(1.2*x)'
[]
[forcing_rho_et]
  type = ParsedFunction
  value = '1.0*eps*(3.48788261470924*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x) + 26.7439413073546*cos(1.2*x))*sin(x)*cos(1.1*x)/cos(x)^2 - 1.1*eps*(3.48788261470924*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x) + 26.7439413073546*cos(1.2*x))*sin(1.1*x)/cos(x) + 1.0*eps*(-(10.6975765229419*cos(1.2*x)/cos(x) - 0.697576522941849*cos(1.1*x)^2/cos(x)^2)*sin(x) + (10.6975765229419*sin(x)*cos(1.2*x)/cos(x)^2 - 1.3951530458837*sin(x)*cos(1.1*x)^2/cos(x)^3 + 1.53466835047207*sin(1.1*x)*cos(1.1*x)/cos(x)^2 - 12.8370918275302*sin(1.2*x)/cos(x))*cos(x) - 32.0927295688256*sin(1.2*x))*cos(1.1*x)/cos(x)'
  vars = 'eps'
  vals = '${eps}'
[]
[exact_T]
  type = ParsedFunction
  value = '0.0106975765229418*cos(1.2*x)/cos(x) - 0.000697576522941848*cos(1.1*x)^2/cos(x)^2'
  vars = 'eps'
  vals = '${eps}'
[]
[exact_eps_p]
  type = ParsedFunction
  value = '3.48788261470924*eps*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x)'
  vars = 'eps'
  vals = '${eps}'
[]
[]

[Executioner]
  solve_type = NEWTON
  type = Steady
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       superlu_dist'
  nl_max_its = 50
  line_search = none
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
  csv = true
[]

[Debug]
  show_var_residual_norms = true
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2rho]
    type = ElementL2Error
    variable = rho
    function = exact_rho
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2rho_ud]
    variable = rho_ud
    function = exact_rho_ud
    type = ElementL2Error
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2rho_et]
    variable = rho_et
    function = exact_rho_et
    type = ElementL2Error
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]
