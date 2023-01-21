[GlobalParams]
  fp = fp
[]

[Mesh]
  [cartesian]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = .1
    xmax = .6
    nx = 2
  []
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[Problem]
  fv_bcs_integrity_check = false
[]

[Variables]
  [pressure]
    type = MooseVariableFVReal
  []
  [sup_mom_x]
    type = MooseVariableFVReal
  []
  [T_fluid]
    type = MooseVariableFVReal
  []
[]

[ICs]
  [pressure]
    type = FunctionIC
    variable = pressure
    function = 'exact_p'
  []
  [sup_mom_x]
    type = FunctionIC
    variable = sup_mom_x
    function = 'exact_rho_ud'
  []
  [T_fluid]
    type = FunctionIC
    variable = T_fluid
    function = 'exact_T'
  []
[]

[FVKernels]
  [mass_advection]
    type = PCNSFVMassHLLC
    variable = pressure
  []
  [mass_fn]
    type = FVBodyForce
    variable = pressure
    function = 'forcing_rho'
  []

  [momentum_x_advection]
    type = PCNSFVMomentumHLLC
    variable = sup_mom_x
    momentum_component = x
  []
  [eps_grad]
    type = PNSFVPGradEpsilon
    variable = sup_mom_x
    momentum_component = 'x'
    epsilon_function = 'eps'
  []
  [momentum_fn]
    type = FVBodyForce
    variable = sup_mom_x
    function = 'forcing_rho_ud'
  []

  [fluid_energy_advection]
    type = PCNSFVFluidEnergyHLLC
    variable = T_fluid
  []
  [energy_fn]
    type = FVBodyForce
    variable = T_fluid
    function = 'forcing_rho_et'
  []
[]

[FVBCs]
  [mass_left]
    variable = pressure
    type = PCNSFVStrongBC
    boundary = left
    T_fluid = 'exact_T'
    superficial_velocity = 'exact_superficial_velocity'
    eqn = 'mass'
  []
  [momentum_left]
    variable = sup_mom_x
    type = PCNSFVStrongBC
    boundary = left
    T_fluid = 'exact_T'
    superficial_velocity = 'exact_superficial_velocity'
    eqn = 'momentum'
    momentum_component = 'x'
  []
  [energy_left]
    variable = T_fluid
    type = PCNSFVStrongBC
    boundary = left
    T_fluid = 'exact_T'
    superficial_velocity = 'exact_superficial_velocity'
    eqn = 'energy'
  []
  [mass_right]
    variable = pressure
    type = PCNSFVStrongBC
    boundary = right
    eqn = 'mass'
    pressure = 'exact_p'
  []
  [momentum_right]
    variable = sup_mom_x
    type = PCNSFVStrongBC
    boundary = right
    eqn = 'momentum'
    momentum_component = 'x'
    pressure = 'exact_p'
  []
  [energy_right]
    variable = T_fluid
    type = PCNSFVStrongBC
    boundary = right
    eqn = 'energy'
    pressure = 'exact_p'
  []
[]

[Materials]
  [var_mat]
    type = PorousMixedVarMaterial
    pressure = pressure
    superficial_rhou = sup_mom_x
    T_fluid = T_fluid
    porosity = porosity
  []
  [porosity]
    type = GenericFunctionMaterial
    prop_names = 'porosity'
    prop_values = 'eps'
  []
[]

[Functions]
[exact_rho]
  type = ParsedFunction
  expression = '3.48788261470924*cos(x)'
[]
[forcing_rho]
  type = ParsedFunction
  expression = '-3.83667087618017*sin(1.1*x)*cos(1.3*x) - 4.53424739912202*sin(1.3*x)*cos(1.1*x)'
[]
[exact_rho_ud]
  type = ParsedFunction
  expression = '3.48788261470924*cos(1.1*x)*cos(1.3*x)'
[]
[forcing_rho_ud]
  type = ParsedFunction
  expression = '(-(10.6975765229419*cos(1.5*x)/cos(x) - 0.697576522941849*cos(1.1*x)^2/cos(x)^2)*sin(x) + (10.6975765229419*sin(x)*cos(1.5*x)/cos(x)^2 - 1.3951530458837*sin(x)*cos(1.1*x)^2/cos(x)^3 + 1.53466835047207*sin(1.1*x)*cos(1.1*x)/cos(x)^2 - 16.0463647844128*sin(1.5*x)/cos(x))*cos(x))*cos(1.3*x) + 3.48788261470924*sin(x)*cos(1.1*x)^2*cos(1.3*x)/cos(x)^2 - 7.67334175236034*sin(1.1*x)*cos(1.1*x)*cos(1.3*x)/cos(x) - 4.53424739912202*sin(1.3*x)*cos(1.1*x)^2/cos(x)'
[]
[exact_rho_et]
  type = ParsedFunction
  expression = '26.7439413073546*cos(1.5*x)'
[]
[forcing_rho_et]
  type = ParsedFunction
  expression = '1.0*(3.48788261470924*(3.06706896551724*cos(1.5*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x) + 26.7439413073546*cos(1.5*x))*sin(x)*cos(1.1*x)*cos(1.3*x)/cos(x)^2 - 1.1*(3.48788261470924*(3.06706896551724*cos(1.5*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x) + 26.7439413073546*cos(1.5*x))*sin(1.1*x)*cos(1.3*x)/cos(x) - 1.3*(3.48788261470924*(3.06706896551724*cos(1.5*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x) + 26.7439413073546*cos(1.5*x))*sin(1.3*x)*cos(1.1*x)/cos(x) + 1.0*(-(10.6975765229419*cos(1.5*x)/cos(x) - 0.697576522941849*cos(1.1*x)^2/cos(x)^2)*sin(x) + (10.6975765229419*sin(x)*cos(1.5*x)/cos(x)^2 - 1.3951530458837*sin(x)*cos(1.1*x)^2/cos(x)^3 + 1.53466835047207*sin(1.1*x)*cos(1.1*x)/cos(x)^2 - 16.0463647844128*sin(1.5*x)/cos(x))*cos(x) - 40.1159119610319*sin(1.5*x))*cos(1.1*x)*cos(1.3*x)/cos(x)'
[]
[exact_T]
  type = ParsedFunction
  expression = '0.0106975765229418*cos(1.5*x)/cos(x) - 0.000697576522941848*cos(1.1*x)^2/cos(x)^2'
[]
[exact_eps_p]
  type = ParsedFunction
  expression = '3.48788261470924*(3.06706896551724*cos(1.5*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x)*cos(1.3*x)'
[]
[exact_p]
  type = ParsedFunction
  expression = '3.48788261470924*(3.06706896551724*cos(1.5*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x)'
[]
[exact_sup_vel_x]
  type = ParsedFunction
  expression = '1.0*cos(1.1*x)*cos(1.3*x)/cos(x)'
[]
[eps]
  type = ParsedFunction
  expression = 'cos(1.3*x)'
[]
[exact_superficial_velocity]
  type = ParsedVectorFunction
  expression_x = '1.0*cos(1.1*x)*cos(1.3*x)/cos(x)'
[]
[]

[Executioner]
  solve_type = NEWTON
  type = Steady
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_max_its = 50
  line_search = bt
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
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
  [L2pressure]
    type = ElementL2Error
    variable = pressure
    function = exact_p
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2sup_mom_x]
    variable = sup_mom_x
    function = exact_rho_ud
    type = ElementL2Error
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2T_fluid]
    variable = T_fluid
    function = exact_T
    type = ElementL2Error
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]
