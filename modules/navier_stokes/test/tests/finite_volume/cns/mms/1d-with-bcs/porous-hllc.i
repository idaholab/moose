eps=0.9

[GlobalParams]
  fp = fp
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

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
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
    type = PCNSFVMassHLLC
    variable = rho
    fp = fp
  []
  [mass_fn]
    type = FVBodyForce
    variable = rho
    function = 'forcing_rho'
  []

  [momentum_x_advection]
    type = PCNSFVMomentumHLLC
    variable = rho_ud
    momentum_component = x
    fp = fp
  []
  [momentum_fn]
    type = FVBodyForce
    variable = rho_ud
    function = 'forcing_rho_ud'
  []

  [fluid_energy_advection]
    type = PCNSFVFluidEnergyHLLC
    variable = rho_et
    fp = fp
  []
  [energy_fn]
    type = FVBodyForce
    variable = rho_et
    function = 'forcing_rho_et'
  []
[]

[FVBCs]
  [mass_in]
    variable = rho
    type = PCNSFVHLLCSpecifiedMassFluxAndTemperatureMassBC
    boundary = left
    temperature = 'exact_T'
    superficial_rhou = 'exact_rho_ud'
  []
  [momentum_in]
    variable = rho_ud
    type = PCNSFVHLLCSpecifiedMassFluxAndTemperatureMomentumBC
    boundary = left
    temperature = 'exact_T'
    superficial_rhou = 'exact_rho_ud'
    momentum_component = 'x'
  []
  [energy_in]
    variable = rho_et
    type = PCNSFVHLLCSpecifiedMassFluxAndTemperatureFluidEnergyBC
    boundary = left
    temperature = 'exact_T'
    superficial_rhou = 'exact_rho_ud'
  []

  [mass_out]
    variable = rho
    type = PCNSFVHLLCSpecifiedPressureMassBC
    boundary = right
    pressure = 'exact_p'
  []
  [momentum_out]
    variable = rho_ud
    type = PCNSFVHLLCSpecifiedPressureMomentumBC
    boundary = right
    pressure = 'exact_p'
    momentum_component = 'x'
  []
  [energy_out]
    variable = rho_et
    type = PCNSFVHLLCSpecifiedPressureFluidEnergyBC
    boundary = right
    pressure = 'exact_p'
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
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '${eps}'
  []
[]

[Functions]
[exact_rho]
  type = ParsedFunction
  expression = '3.48788261470924*cos(x)'
[]
[forcing_rho]
  type = ParsedFunction
  expression = '-3.83667087618017*eps*sin(1.1*x)'
  symbol_names = 'eps'
  symbol_values = '${eps}'
[]
[exact_rho_ud]
  type = ParsedFunction
  expression = '3.48788261470924*eps*cos(1.1*x)'
  symbol_names = 'eps'
  symbol_values = '${eps}'
[]
[forcing_rho_ud]
  type = ParsedFunction
  expression = 'eps*(-(10.6975765229419*cos(1.2*x)/cos(x) - 0.697576522941849*cos(1.1*x)^2/cos(x)^2)*sin(x) + (10.6975765229419*sin(x)*cos(1.2*x)/cos(x)^2 - 1.3951530458837*sin(x)*cos(1.1*x)^2/cos(x)^3 + 1.53466835047207*sin(1.1*x)*cos(1.1*x)/cos(x)^2 - 12.8370918275302*sin(1.2*x)/cos(x))*cos(x)) + 3.48788261470924*eps*sin(x)*cos(1.1*x)^2/cos(x)^2 - 7.67334175236034*eps*sin(1.1*x)*cos(1.1*x)/cos(x)'
  symbol_names = 'eps'
  symbol_values = '${eps}'
[]
[exact_rho_et]
  type = ParsedFunction
  expression = '26.7439413073546*cos(1.2*x)'
[]
[forcing_rho_et]
  type = ParsedFunction
  expression = '1.0*eps*(3.48788261470924*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x) + 26.7439413073546*cos(1.2*x))*sin(x)*cos(1.1*x)/cos(x)^2 - 1.1*eps*(3.48788261470924*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x) + 26.7439413073546*cos(1.2*x))*sin(1.1*x)/cos(x) + 1.0*eps*(-(10.6975765229419*cos(1.2*x)/cos(x) - 0.697576522941849*cos(1.1*x)^2/cos(x)^2)*sin(x) + (10.6975765229419*sin(x)*cos(1.2*x)/cos(x)^2 - 1.3951530458837*sin(x)*cos(1.1*x)^2/cos(x)^3 + 1.53466835047207*sin(1.1*x)*cos(1.1*x)/cos(x)^2 - 12.8370918275302*sin(1.2*x)/cos(x))*cos(x) - 32.0927295688256*sin(1.2*x))*cos(1.1*x)/cos(x)'
  symbol_names = 'eps'
  symbol_values = '${eps}'
[]
[exact_T]
  type = ParsedFunction
  expression = '0.0106975765229418*cos(1.2*x)/cos(x) - 0.000697576522941848*cos(1.1*x)^2/cos(x)^2'
  symbol_names = 'eps'
  symbol_values = '${eps}'
[]
[exact_p]
  type = ParsedFunction
  expression = '3.48788261470924*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x)'
  symbol_names = 'eps'
  symbol_values = '${eps}'
[]
[]

[Executioner]
  solve_type = NEWTON
  type = Steady
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_max_its = 50
  line_search = none
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
