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
  [rho_u]
    type = FunctionIC
    variable = rho_u
    function = 'exact_rho_u'
  []
  [rho_et]
    type = FunctionIC
    variable = rho_et
    function = 'exact_rho_et'
  []
[]

[FVKernels]
  [mass_advection]
    type = CNSFVMassHLLC
    variable = rho
    fp = fp
  []
  [mass_fn]
    type = FVBodyForce
    variable = rho
    function = 'forcing_rho'
  []

  [momentum_x_advection]
    type = CNSFVMomentumHLLC
    variable = rho_u
    momentum_component = x
    fp = fp
  []
  [momentum_fn]
    type = FVBodyForce
    variable = rho_u
    function = 'forcing_rho_u'
  []

  [fluid_energy_advection]
    type = CNSFVFluidEnergyHLLC
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
    type = CNSFVHLLCSpecifiedMassFluxAndTemperatureMassBC
    boundary = left
    temperature = 'exact_T'
    rhou = 'exact_rho_u'
  []
  [momentum_in]
    variable = rho_u
    type = CNSFVHLLCSpecifiedMassFluxAndTemperatureMomentumBC
    boundary = left
    temperature = 'exact_T'
    rhou = 'exact_rho_u'
    momentum_component = 'x'
  []
  [energy_in]
    variable = rho_et
    type = CNSFVHLLCSpecifiedMassFluxAndTemperatureFluidEnergyBC
    boundary = left
    temperature = 'exact_T'
    rhou = 'exact_rho_u'
  []

  [mass_out]
    variable = rho
    type = CNSFVHLLCSpecifiedPressureMassBC
    boundary = right
    pressure = 'exact_p'
  []
  [momentum_out]
    variable = rho_u
    type = CNSFVHLLCSpecifiedPressureMomentumBC
    boundary = right
    pressure = 'exact_p'
    momentum_component = 'x'
  []
  [energy_out]
    variable = rho_et
    type = CNSFVHLLCSpecifiedPressureFluidEnergyBC
    boundary = right
    pressure = 'exact_p'
  []
[]

[Materials]
  [var_mat]
    type = ConservedVarValuesMaterial
    rho = rho
    rhou = rho_u
    rho_et = rho_et
  []
[]

[Functions]
[exact_rho]
  type = ParsedFunction
  value = '3.48788261470924*cos(x)'
[]
[forcing_rho]
  type = ParsedFunction
  value = '-3.83667087618017*sin(1.1*x)'
[]
[exact_rho_u]
  type = ParsedFunction
  value = '3.48788261470924*cos(1.1*x)'
[]
[forcing_rho_u]
  type = ParsedFunction
  value = '-(10.6975765229419*cos(1.2*x)/cos(x) - 0.697576522941849*cos(1.1*x)^2/cos(x)^2)*sin(x) + (10.6975765229419*sin(x)*cos(1.2*x)/cos(x)^2 - 1.3951530458837*sin(x)*cos(1.1*x)^2/cos(x)^3 + 1.53466835047207*sin(1.1*x)*cos(1.1*x)/cos(x)^2 - 12.8370918275302*sin(1.2*x)/cos(x))*cos(x) + 3.48788261470924*sin(x)*cos(1.1*x)^2/cos(x)^2 - 7.67334175236034*sin(1.1*x)*cos(1.1*x)/cos(x)'
[]
[exact_rho_et]
  type = ParsedFunction
  value = '26.7439413073546*cos(1.2*x)'
[]
[forcing_rho_et]
  type = ParsedFunction
  value = '1.0*(3.48788261470924*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x) + 26.7439413073546*cos(1.2*x))*sin(x)*cos(1.1*x)/cos(x)^2 - 1.1*(3.48788261470924*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x) + 26.7439413073546*cos(1.2*x))*sin(1.1*x)/cos(x) + 1.0*(-(10.6975765229419*cos(1.2*x)/cos(x) - 0.697576522941849*cos(1.1*x)^2/cos(x)^2)*sin(x) + (10.6975765229419*sin(x)*cos(1.2*x)/cos(x)^2 - 1.3951530458837*sin(x)*cos(1.1*x)^2/cos(x)^3 + 1.53466835047207*sin(1.1*x)*cos(1.1*x)/cos(x)^2 - 12.8370918275302*sin(1.2*x)/cos(x))*cos(x) - 32.0927295688256*sin(1.2*x))*cos(1.1*x)/cos(x)'
[]
[exact_T]
  type = ParsedFunction
  value = '0.0106975765229418*cos(1.2*x)/cos(x) - 0.000697576522941848*cos(1.1*x)^2/cos(x)^2'
[]
[exact_p]
  type = ParsedFunction
  value = '3.48788261470924*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x)'
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
  [L2rho_u]
    variable = rho_u
    function = exact_rho_u
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
