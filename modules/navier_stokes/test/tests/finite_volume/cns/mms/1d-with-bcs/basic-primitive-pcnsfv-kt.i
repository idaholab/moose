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
  [pressure]
    type = MooseVariableFVReal
  []
  [sup_vel_x]
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
  [sup_vel_x]
    type = FunctionIC
    variable = sup_vel_x
    function = 'exact_sup_vel_x'
  []
  [T_fluid]
    type = FunctionIC
    variable = T_fluid
    function = 'exact_T'
  []
[]

[FVKernels]
  [mass_advection]
    type = PCNSFVKT
    variable = pressure
    eqn = "mass"
  []
  [mass_fn]
    type = FVBodyForce
    variable = pressure
    function = 'forcing_rho'
  []

  [momentum_x_advection]
    type = PCNSFVKT
    variable = sup_vel_x
    momentum_component = x
    eqn = "momentum"
  []
  [momentum_fn]
    type = FVBodyForce
    variable = sup_vel_x
    function = 'forcing_rho_ud'
  []

  [fluid_energy_advection]
    type = PCNSFVKT
    variable = T_fluid
    eqn = "energy"
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
    variable = sup_vel_x
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
    variable = sup_vel_x
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

  # help gradient reconstruction
  [pressure_right]
    type = FVFunctionDirichletBC
    variable = pressure
    function = exact_p
    boundary = 'right'
  []
  [sup_vel_x_left]
    type = FVFunctionDirichletBC
    variable = sup_vel_x
    function = exact_sup_vel_x
    boundary = 'left'
  []
  [T_fluid_left]
    type = FVFunctionDirichletBC
    variable = T_fluid
    function = exact_T
    boundary = 'left'
  []
[]

[Materials]
  [var_mat]
    type = PorousPrimitiveVarMaterial
    pressure = pressure
    superficial_vel_x = sup_vel_x
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

[Executioner]
  solve_type = NEWTON
  type = Transient
  num_steps = 1
  dtmin = 1
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
  [L2sup_vel_x]
    variable = sup_vel_x
    function = exact_sup_vel_x
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
