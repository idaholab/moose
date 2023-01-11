[GlobalParams]
  fp = fp
  limiter = 'central_difference'
  two_term_boundary_expansion = true
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
  [pressure]
    type = FunctionIC
    variable = rho
    function = 'exact_rho'
  []
  [sup_vel_x]
    type = FunctionIC
    variable = rho_ud
    function = 'exact_rho_ud'
  []
  [T_fluid]
    type = FunctionIC
    variable = rho_et
    function = 'exact_rho_et'
  []
[]

[FVKernels]
  [mass_advection]
    type = PCNSFVKT
    variable = rho
    eqn = "mass"
  []
  [mass_fn]
    type = FVBodyForce
    variable = rho
    function = 'forcing_rho'
  []

  [momentum_x_advection]
    type = PCNSFVKT
    variable = rho_ud
    momentum_component = x
    eqn = "momentum"
  []
  [momentum_fn]
    type = FVBodyForce
    variable = rho_ud
    function = 'forcing_rho_ud'
  []

  [fluid_energy_advection]
    type = PCNSFVKT
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
    type = PCNSFVStrongBC
    boundary = left
    T_fluid = 'exact_T'
    superficial_velocity = 'exact_superficial_velocity'
    eqn = 'mass'
  []
  [momentum_left]
    variable = rho_ud
    type = PCNSFVStrongBC
    boundary = left
    T_fluid = 'exact_T'
    superficial_velocity = 'exact_superficial_velocity'
    eqn = 'momentum'
    momentum_component = 'x'
  []
  [energy_left]
    variable = rho_et
    type = PCNSFVStrongBC
    boundary = left
    T_fluid = 'exact_T'
    superficial_velocity = 'exact_superficial_velocity'
    eqn = 'energy'
  []
  [mass_right]
    variable = rho
    type = PCNSFVStrongBC
    boundary = right
    eqn = 'mass'
    pressure = 'exact_p'
  []
  [momentum_right]
    variable = rho_ud
    type = PCNSFVStrongBC
    boundary = right
    eqn = 'momentum'
    momentum_component = 'x'
    pressure = 'exact_p'
  []
  [energy_right]
    variable = rho_et
    type = PCNSFVStrongBC
    boundary = right
    eqn = 'energy'
    pressure = 'exact_p'
  []

  # help gradient reconstruction
  [rho_right]
    type = FVFunctionDirichletBC
    variable = rho
    function = exact_rho
    boundary = 'right'
  []
  [rho_ud_left]
    type = FVFunctionDirichletBC
    variable = rho_ud
    function = exact_rho_ud
    boundary = 'left'
  []
  [rho_et_left]
    type = FVFunctionDirichletBC
    variable = rho_et
    function = exact_rho_et
    boundary = 'left'
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
  expression = '3.48788261470924*cos(x)'
[]
[forcing_rho]
  type = ParsedFunction
  expression = '-3.45300378856215*sin(1.1*x)'
[]
[exact_rho_ud]
  type = ParsedFunction
  expression = '3.13909435323832*cos(1.1*x)'
[]
[forcing_rho_ud]
  type = ParsedFunction
  expression = '-0.9*(10.6975765229419*cos(1.2*x)/cos(x) - 0.697576522941849*cos(1.1*x)^2/cos(x)^2)*sin(x) + 0.9*(10.6975765229419*sin(x)*cos(1.2*x)/cos(x)^2 - 1.3951530458837*sin(x)*cos(1.1*x)^2/cos(x)^3 + 1.53466835047207*sin(1.1*x)*cos(1.1*x)/cos(x)^2 - 12.8370918275302*sin(1.2*x)/cos(x))*cos(x) + 3.13909435323832*sin(x)*cos(1.1*x)^2/cos(x)^2 - 6.9060075771243*sin(1.1*x)*cos(1.1*x)/cos(x)'
[]
[exact_rho_et]
  type = ParsedFunction
  expression = '26.7439413073546*cos(1.2*x)'
[]
[forcing_rho_et]
  type = ParsedFunction
  expression = '0.9*(3.48788261470924*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x) + 26.7439413073546*cos(1.2*x))*sin(x)*cos(1.1*x)/cos(x)^2 - 0.99*(3.48788261470924*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x) + 26.7439413073546*cos(1.2*x))*sin(1.1*x)/cos(x) + 0.9*(-(10.6975765229419*cos(1.2*x)/cos(x) - 0.697576522941849*cos(1.1*x)^2/cos(x)^2)*sin(x) + (10.6975765229419*sin(x)*cos(1.2*x)/cos(x)^2 - 1.3951530458837*sin(x)*cos(1.1*x)^2/cos(x)^3 + 1.53466835047207*sin(1.1*x)*cos(1.1*x)/cos(x)^2 - 12.8370918275302*sin(1.2*x)/cos(x))*cos(x) - 32.0927295688256*sin(1.2*x))*cos(1.1*x)/cos(x)'
[]
[exact_T]
  type = ParsedFunction
  expression = '0.0106975765229418*cos(1.2*x)/cos(x) - 0.000697576522941848*cos(1.1*x)^2/cos(x)^2'
[]
[exact_eps_p]
  type = ParsedFunction
  expression = '3.13909435323832*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x)'
[]
[exact_p]
  type = ParsedFunction
  expression = '3.48788261470924*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x)'
[]
[exact_sup_vel_x]
  type = ParsedFunction
  expression = '0.9*cos(1.1*x)/cos(x)'
[]
[exact_superficial_velocity]
  type = ParsedVectorFunction
  expression_x = '0.9*cos(1.1*x)/cos(x)'
[]
[eps]
  type = ParsedFunction
  expression = '0.9'
[]
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  solve_type = NEWTON
  type = Transient
  num_steps = 1
  dtmin = 1
  petsc_options = '-snes_linesearch_monitor'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_max_its = 50
  line_search = bt
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
