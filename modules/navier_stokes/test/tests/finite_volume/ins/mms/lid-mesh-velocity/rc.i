mu=1.1
rho=1.1

[GlobalParams]
  rhie_chow_user_object = 'rc'
  velocity_interp_method = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = u
    v = v
    pressure = pressure
    disp_x = disp_x
    disp_y = disp_y
    use_displaced_mesh = true
  []
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
    nx = 2
    ny = 2
  []
  displacements = 'disp_x disp_y'
[]

[Problem]
  fv_bcs_integrity_check = false
  error_on_jacobian_nonzero_reallocation = true
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
    initial_condition = 1
  []
  [v]
    type = INSFVVelocityVariable
    initial_condition = 1
  []
  [pressure]
    type = INSFVPressureVariable
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]

[AuxVariables]
  [disp_x][]
  [disp_y][]
[]

[AuxKernels]
  [disp_x]
    type = FunctionAux
    function = exact_disp_x
    variable = disp_x
    execute_on = 'initial timestep_begin'
  []
  [disp_y]
    type = FunctionAux
    function = exact_disp_y
    variable = disp_y
    execute_on = 'initial timestep_begin'
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = 'average'
    rho = ${rho}
    use_displaced_mesh = true
  []
  [mass_forcing]
    type = FVBodyForce
    variable = pressure
    function = forcing_p
    use_displaced_mesh = true
  []
  [mean_zero_pressure]
    type = FVIntegralValueConstraint
    variable = pressure
    lambda = lambda
    use_displaced_mesh = true
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    advected_interp_method = 'average'
    rho = ${rho}
    momentum_component = 'x'
    use_displaced_mesh = true
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = u
    mu = ${mu}
    momentum_component = 'x'
    use_displaced_mesh = true
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    pressure = pressure
    use_displaced_mesh = true
  []
  [u_forcing]
    type = INSFVBodyForce
    variable = u
    functor = forcing_u
    momentum_component = 'x'
    use_displaced_mesh = true
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = v
    advected_interp_method = 'average'
    rho = ${rho}
    momentum_component = 'y'
    use_displaced_mesh = true
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = v
    mu = ${mu}
    momentum_component = 'y'
    use_displaced_mesh = true
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    pressure = pressure
    use_displaced_mesh = true
  []
  [v_forcing]
    type = INSFVBodyForce
    variable = v
    functor = forcing_v
    momentum_component = 'y'
    use_displaced_mesh = true
  []
[]

[FVBCs]
  [no-slip-wall-u]
    type = INSFVNoSlipWallBC
    boundary = 'left right top bottom'
    variable = u
    function = 'exact_u'
  []
  [no-slip-wall-v]
    type = INSFVNoSlipWallBC
    boundary = 'left right top bottom'
    variable = v
    function = 'exact_v'
  []
  [pressure]
    type = FVFunctionDirichletBC
    boundary = 'left right top bottom'
    variable = pressure
    function = 'exact_p'
  []
[]

[Functions]
[exact_u]
  type = ParsedFunction
  expression = 'sin(y)*cos((1/2)*x*pi)'
[]
[exact_rhou]
  type = ParsedFunction
  expression = 'rho*sin(y)*cos((1/2)*x*pi)'
  symbol_names = 'rho'
  symbol_values = '${rho}'
[]
[forcing_u]
  type = ParsedFunction
  expression = 'mu*sin(y)*cos((1/2)*x*pi) + (1/4)*pi^2*mu*sin(y)*cos((1/2)*x*pi) - 1/2*pi*rho*sin(x)*sin(y)*sin((1/2)*y*pi)*cos((1/2)*x*pi) + rho*sin(x)*cos(y)*cos((1/2)*x*pi)*cos((1/2)*y*pi) - pi*rho*sin(y)^2*sin((1/2)*x*pi)*cos((1/2)*x*pi) + sin(y)*cos(x)'
  symbol_names = 'mu rho'
  symbol_values = '${mu} ${rho}'
[]
[exact_v]
  type = ParsedFunction
  expression = 'sin(x)*cos((1/2)*y*pi)'
[]
[exact_rhov]
  type = ParsedFunction
  expression = 'rho*sin(x)*cos((1/2)*y*pi)'
  symbol_names = 'rho'
  symbol_values = '${rho}'
[]
[forcing_v]
  type = ParsedFunction
  expression = 'mu*sin(x)*cos((1/2)*y*pi) + (1/4)*pi^2*mu*sin(x)*cos((1/2)*y*pi) - pi*rho*sin(x)^2*sin((1/2)*y*pi)*cos((1/2)*y*pi) - 1/2*pi*rho*sin(x)*sin(y)*sin((1/2)*x*pi)*cos((1/2)*y*pi) + rho*sin(y)*cos(x)*cos((1/2)*x*pi)*cos((1/2)*y*pi) + sin(x)*cos(y)'
  symbol_names = 'mu rho'
  symbol_values = '${mu} ${rho}'
[]
[exact_p]
  type = ParsedFunction
  expression = 'sin(x)*sin(y)'
[]
[forcing_p]
  type = ParsedFunction
  expression = '-1/2*pi*rho*sin(x)*sin((1/2)*y*pi) - 1/2*pi*rho*sin(y)*sin((1/2)*x*pi)'
  symbol_names = 'rho'
  symbol_values = '${rho}'
[]
[exact_disp_x]
  type = ParsedFunction
  expression = '0'
[]
[exact_disp_y]
  type = ParsedFunction
  expression = '0'
[]
[]

[Executioner]
  type = Transient
  num_steps = 1
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  nl_rel_tol = 1e-12
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2u]
    type = ElementL2FunctorError
    approximate = u
    exact = exact_u
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2v]
    approximate = v
    exact = exact_v
    type = ElementL2FunctorError
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2p]
    approximate = pressure
    exact = exact_p
    type = ElementL2FunctorError
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]
