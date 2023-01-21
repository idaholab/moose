diff_coeff = 0.1

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
  []
  [momentum_fn]
    type = FVBodyForce
    variable = rho_u
    function = 'forcing_rho_u'
  []

  [fluid_energy_advection]
    type = CNSFVFluidEnergyHLLC
    variable = rho_et
  []
  [energy_fn]
    type = FVBodyForce
    variable = rho_et
    function = 'forcing_rho_et'
  []
  [mass_diff]
    type = FVDiffusion
    variable = rho
    coeff = ${diff_coeff}
  []
  [momentum_diff]
    type = FVDiffusion
    variable = rho_u
    coeff = ${diff_coeff}
  []
  [energy_diff]
    type = FVDiffusion
    variable = rho_et
    coeff = ${diff_coeff}
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

  [left_mass_diffusion]
    type = FVFunctionNeumannBC
    variable = rho
    function = minus_rho_bc
    boundary = 'left'
  []
  [left_momentum_diffusion]
    type = FVFunctionNeumannBC
    variable = rho_u
    function = minus_rho_u_bc
    boundary = 'left'
  []
  [left_energy_diffusion]
    type = FVFunctionNeumannBC
    variable = rho_et
    function = minus_rho_et_bc
    boundary = 'left'
  []
  [right_mass_diffusion]
    type = FVFunctionNeumannBC
    variable = rho
    function = rho_bc
    boundary = 'right'
  []
  [right_momentum_diffusion]
    type = FVFunctionNeumannBC
    variable = rho_u
    function = rho_u_bc
    boundary = 'right'
  []
  [right_energy_diffusion]
    type = FVFunctionNeumannBC
    variable = rho_et
    function = rho_et_bc
    boundary = 'right'
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
    expression = '3.48788261470924*cos(x)'
  []
  [rho_bc]
    type = ParsedFunction
    value = '-diff_coeff*3.48788261470924*sin(x)'
    vars = 'diff_coeff'
    vals = '${diff_coeff}'
  []
  [minus_rho_bc]
    type = ParsedFunction
    value = 'diff_coeff*3.48788261470924*sin(x)'
    vars = 'diff_coeff'
    vals = '${diff_coeff}'
  []
  [forcing_rho]
    type = ParsedFunction
    expression = '-3.83667087618017*sin(1.1*x) + 0.348788261470924*cos(x)'
  []
  [exact_rho_u]
    type = ParsedFunction
    expression = '3.48788261470924*cos(1.1*x)'
  []
  [rho_u_bc]
    type = ParsedFunction
    value = '-diff_coeff*3.48788261470924*1.1*sin(1.1*x)'
    vars = 'diff_coeff'
    vals = '${diff_coeff}'
  []
  [minus_rho_u_bc]
    type = ParsedFunction
    value = 'diff_coeff*3.48788261470924*1.1*sin(1.1*x)'
    vars = 'diff_coeff'
    vals = '${diff_coeff}'
  []
  [forcing_rho_u]
    type = ParsedFunction
    expression = '-(10.6975765229419*cos(1.2*x)/cos(x) - 0.697576522941849*cos(1.1*x)^2/cos(x)^2)*sin(x) + (10.6975765229419*sin(x)*cos(1.2*x)/cos(x)^2 - 1.3951530458837*sin(x)*cos(1.1*x)^2/cos(x)^3 + 1.53466835047207*sin(1.1*x)*cos(1.1*x)/cos(x)^2 - 12.8370918275302*sin(1.2*x)/cos(x))*cos(x) + 3.48788261470924*sin(x)*cos(1.1*x)^2/cos(x)^2 - 7.67334175236034*sin(1.1*x)*cos(1.1*x)/cos(x) + 0.422033796379819*cos(1.1*x)'
  []
  [exact_rho_et]
    type = ParsedFunction
    expression = '26.7439413073546*cos(1.2*x)'
  []
  [rho_et_bc]
    type = ParsedFunction
    value = '-diff_coeff*26.7439413073546*1.2*sin(1.2*x)'
    vars = 'diff_coeff'
    vals = '${diff_coeff}'
  []
  [minus_rho_et_bc]
    type = ParsedFunction
    value = 'diff_coeff*26.7439413073546*1.2*sin(1.2*x)'
    vars = 'diff_coeff'
    vals = '${diff_coeff}'
  []
  [forcing_rho_et]
    type = ParsedFunction
    expression = '1.0*(3.48788261470924*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x) + 26.7439413073546*cos(1.2*x))*sin(x)*cos(1.1*x)/cos(x)^2 - 1.1*(3.48788261470924*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x) + 26.7439413073546*cos(1.2*x))*sin(1.1*x)/cos(x) + 1.0*(-(10.6975765229419*cos(1.2*x)/cos(x) - 0.697576522941849*cos(1.1*x)^2/cos(x)^2)*sin(x) + (10.6975765229419*sin(x)*cos(1.2*x)/cos(x)^2 - 1.3951530458837*sin(x)*cos(1.1*x)^2/cos(x)^3 + 1.53466835047207*sin(1.1*x)*cos(1.1*x)/cos(x)^2 - 12.8370918275302*sin(1.2*x)/cos(x))*cos(x) - 32.0927295688256*sin(1.2*x))*cos(1.1*x)/cos(x) + 3.85112754825907*cos(1.2*x)'
  []
  [exact_T]
    type = ParsedFunction
    expression = '0.0106975765229418*cos(1.2*x)/cos(x) - 0.000697576522941848*cos(1.1*x)^2/cos(x)^2'
  []
  [exact_p]
    type = ParsedFunction
    expression = '3.48788261470924*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x)'
  []
[]

[Executioner]
  solve_type = NEWTON
  type = Steady
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_max_its = 50
  line_search = none
  nl_rel_tol = 1e-11
  nl_abs_tol = 1e-11
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
