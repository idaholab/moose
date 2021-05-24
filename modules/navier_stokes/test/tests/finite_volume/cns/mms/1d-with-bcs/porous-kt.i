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
  [rho_ud]
    type = MooseVariableFVReal
  []
  [rho_et]
    type = MooseVariableFVReal
  []
[]

[ICs]
  [rho]
    type = FunctionIC
    variable = rho
    function = 'exact_rho'
  []
  [rho_ud]
    type = FunctionIC
    variable = rho_ud
    function = 'exact_rho_ud'
  []
  [rho_et]
    type = FunctionIC
    variable = rho_et
    function = 'exact_rho_et'
  []
[]

[FVKernels]
  [mass_advection]
    type = PCNSFVInterpolatedLaxFriedrichs
    variable = rho
    eqn = "mass"
  []
  [mass_fn]
    type = FVBodyForce
    variable = rho
    function = 'forcing_rho'
  []

  [momentum_x_advection]
    type = PCNSFVInterpolatedLaxFriedrichs
    variable = rho_ud
    momentum_component = x
    eqn = "momentum"
  []
  [friction]
    type = FVReaction
    variable = rho_ud
  []
  [eps_grad]
    type = PNSFVPGradEpsilon
    variable = rho_ud
    momentum_component = 'x'
    epsilon_function = 'eps'
  []
  [momentum_fn]
    type = FVBodyForce
    variable = rho_ud
    function = 'forcing_rho_ud'
  []

  [fluid_energy_advection]
    type = PCNSFVInterpolatedLaxFriedrichs
    variable = rho_et
    eqn = "energy"
  []
  [energy_fn]
    type = FVBodyForce
    variable = rho_et
    function = 'forcing_rho_et'
  []
[]

[FVBCs]
  [mass_left]
    variable = rho
    type = PCNSFVInterpolatedLaxFriedrichsBC
    boundary = left
    T_fluid = 'exact_T'
    superficial_velocity = 'exact_superficial_velocity'
    eqn = 'mass'
  []
  [momentum_left]
    variable = rho_ud
    type = PCNSFVInterpolatedLaxFriedrichsBC
    boundary = left
    T_fluid = 'exact_T'
    superficial_velocity = 'exact_superficial_velocity'
    eqn = 'momentum'
    momentum_component = 'x'
  []
  [energy_left]
    variable = rho_et
    type = PCNSFVInterpolatedLaxFriedrichsBC
    boundary = left
    T_fluid = 'exact_T'
    superficial_velocity = 'exact_superficial_velocity'
    eqn = 'energy'
  []
  [mass_right]
    variable = rho
    type = PCNSFVInterpolatedLaxFriedrichsBC
    boundary = right
    p = 'exact_p'
    eqn = 'mass'
  []
  [momentum_right]
    variable = rho_ud
    type = PCNSFVInterpolatedLaxFriedrichsBC
    boundary = right
    p = 'exact_p'
    eqn = 'momentum'
    momentum_component = 'x'
  []
  [energy_right]
    variable = rho_et
    type = PCNSFVInterpolatedLaxFriedrichsBC
    boundary = right
    p = 'exact_p'
    eqn = 'energy'
  []
[]

[Materials]
  [var_mat]
    type = PorousConservedVarMaterial
    rho = rho
    superficial_rhou = rho_ud
    rho_et = rho_et
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
  value = '3.48788261470924*cos(x)'
[]
[forcing_rho]
  type = ParsedFunction
  value = '-3.83667087618017*sin(1.1*x)*cos(1.3*x) - 4.53424739912202*sin(1.3*x)*cos(1.1*x)'
[]
[exact_rho_ud]
  type = ParsedFunction
  value = '3.48788261470924*cos(1.1*x)*cos(1.3*x)'
[]
[forcing_rho_ud]
  type = ParsedFunction
  value = '(-(10.6975765229419*cos(1.2*x)/cos(x) - 0.697576522941849*cos(1.1*x)^2/cos(x)^2)*sin(x) + (10.6975765229419*sin(x)*cos(1.2*x)/cos(x)^2 - 1.3951530458837*sin(x)*cos(1.1*x)^2/cos(x)^3 + 1.53466835047207*sin(1.1*x)*cos(1.1*x)/cos(x)^2 - 12.8370918275302*sin(1.2*x)/cos(x))*cos(x))*cos(1.3*x) + 3.48788261470924*sin(x)*cos(1.1*x)^2*cos(1.3*x)/cos(x)^2 - 7.67334175236034*sin(1.1*x)*cos(1.1*x)*cos(1.3*x)/cos(x) - 4.53424739912202*sin(1.3*x)*cos(1.1*x)^2/cos(x) + 3.48788261470924*cos(1.1*x)*cos(1.3*x)'
[]
[exact_rho_et]
  type = ParsedFunction
  value = '26.7439413073546*cos(1.2*x)'
[]
[forcing_rho_et]
  type = ParsedFunction
  value = '1.0*(3.48788261470924*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x) + 26.7439413073546*cos(1.2*x))*sin(x)*cos(1.1*x)*cos(1.3*x)/cos(x)^2 - 1.1*(3.48788261470924*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x) + 26.7439413073546*cos(1.2*x))*sin(1.1*x)*cos(1.3*x)/cos(x) - 1.3*(3.48788261470924*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x) + 26.7439413073546*cos(1.2*x))*sin(1.3*x)*cos(1.1*x)/cos(x) + 1.0*(-(10.6975765229419*cos(1.2*x)/cos(x) - 0.697576522941849*cos(1.1*x)^2/cos(x)^2)*sin(x) + (10.6975765229419*sin(x)*cos(1.2*x)/cos(x)^2 - 1.3951530458837*sin(x)*cos(1.1*x)^2/cos(x)^3 + 1.53466835047207*sin(1.1*x)*cos(1.1*x)/cos(x)^2 - 12.8370918275302*sin(1.2*x)/cos(x))*cos(x) - 32.0927295688256*sin(1.2*x))*cos(1.1*x)*cos(1.3*x)/cos(x)'
[]
[exact_T]
  type = ParsedFunction
  value = '0.0106975765229418*cos(1.2*x)/cos(x) - 0.000697576522941848*cos(1.1*x)^2/cos(x)^2'
[]
[exact_eps_p]
  type = ParsedFunction
  value = '3.48788261470924*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x)*cos(1.3*x)'
[]
[exact_p]
  type = ParsedFunction
  value = '3.48788261470924*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x)'
[]
[exact_superficial_velocity]
  type = ParsedVectorFunction
  value_x = 'cos(1.1*x)*cos(1.3*x)/cos(x)'
[]
[eps]
  type = ParsedFunction
  value = 'cos(1.3*x)'
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
