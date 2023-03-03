mu = 1.1
rho = 1.1
advected_interp_method = 'average'
velocity_interp_method = 'rc'

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 2
    ymin = -1
    ymax = 1
    nx = 2
    ny = 2
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = u
    v = v
    pressure = pressure
  []
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
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
  []
  [mass_forcing]
    type = FVBodyForce
    variable = pressure
    function = forcing_p
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = u
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    pressure = pressure
  []
  [u_forcing]
    type = INSFVBodyForce
    variable = u
    functor = forcing_u
    momentum_component = 'x'
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = v
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = v
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    pressure = pressure
  []
  [v_forcing]
    type = INSFVBodyForce
    variable = v
    functor = forcing_v
    momentum_component = 'y'
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = u
    function = 'exact_u'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = v
    function = 'exact_v'
  []
  [walls-u]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom'
    variable = u
    function = 'exact_u'
  []
  [walls-v]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom'
    variable = v
    function = 'exact_v'
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = 'exact_p'
  []
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    expression = 'sin((1/2)*y*pi)*cos((1/2)*x*pi)'
  []
  [exact_rhou]
    type = ParsedFunction
    expression = 'rho*sin((1/2)*y*pi)*cos((1/2)*x*pi)'
    symbol_names = 'rho'
    symbol_values = '${rho}'
  []
  [forcing_u]
    type = ParsedFunction
    expression = '(1/2)*pi^2*mu*sin((1/2)*y*pi)*cos((1/2)*x*pi) - '
            '1/2*pi*rho*sin((1/4)*x*pi)*sin((1/2)*y*pi)^2*cos((1/2)*x*pi) + '
            '(1/2)*pi*rho*sin((1/4)*x*pi)*cos((1/2)*x*pi)*cos((1/2)*y*pi)^2 - '
            'pi*rho*sin((1/2)*x*pi)*sin((1/2)*y*pi)^2*cos((1/2)*x*pi) - '
            '1/4*pi*sin((1/4)*x*pi)*sin((3/2)*y*pi)'
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
  []
  [exact_v]
    type = ParsedFunction
    expression = 'sin((1/4)*x*pi)*cos((1/2)*y*pi)'
  []
  [exact_rhov]
    type = ParsedFunction
    expression = 'rho*sin((1/4)*x*pi)*cos((1/2)*y*pi)'
    symbol_names = 'rho'
    symbol_values = '${rho}'
  []
  [forcing_v]
    type = ParsedFunction
    expression = '(5/16)*pi^2*mu*sin((1/4)*x*pi)*cos((1/2)*y*pi) - '
            'pi*rho*sin((1/4)*x*pi)^2*sin((1/2)*y*pi)*cos((1/2)*y*pi) - '
            '1/2*pi*rho*sin((1/4)*x*pi)*sin((1/2)*x*pi)*sin((1/2)*y*pi)*cos((1/2)*y*pi) + '
            '(1/4)*pi*rho*sin((1/2)*y*pi)*cos((1/4)*x*pi)*cos((1/2)*x*pi)*cos((1/2)*y*pi) + '
            '(3/2)*pi*cos((1/4)*x*pi)*cos((3/2)*y*pi)'
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
  []
  [exact_p]
    type = ParsedFunction
    expression = 'sin((3/2)*y*pi)*cos((1/4)*x*pi)'
  []
  [forcing_p]
    type = ParsedFunction
    expression = '-1/2*pi*rho*sin((1/4)*x*pi)*sin((1/2)*y*pi) - '
            '1/2*pi*rho*sin((1/2)*x*pi)*sin((1/2)*y*pi)'
    symbol_names = 'rho'
    symbol_values = '${rho}'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       NONZERO               superlu_dist'
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
    type = ElementL2FunctorError
    approximate = v
    exact = exact_v
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2p]
    type = ElementL2FunctorError
    approximate = pressure
    exact = exact_p
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]
