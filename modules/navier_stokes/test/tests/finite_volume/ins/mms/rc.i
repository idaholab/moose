mu=1.1
rho=1.1

[GlobalParams]
  force_boundary_execution = false
  two_term_boundary_expansion = false
  rhie_chow_user_object = 'rc'
  velocity_interp_method = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = u
    v = v
    pressure = pressure
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

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = 'average'
    rho = ${rho}
  []
  [mass_forcing]
    type = FVBodyForce
    variable = pressure
    function = forcing_p
  []
  [mean_zero_pressure]
    type = FVScalarLagrangeMultiplier
    variable = pressure
    lambda = lambda
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    advected_interp_method = 'average'
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
    advected_interp_method = 'average'
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
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    value = 'sin(y)*cos((1/2)*x*pi)'
  []
  [exact_rhou]
    type = ParsedFunction
    value = 'rho*sin(y)*cos((1/2)*x*pi)'
    vars = 'rho'
    vals = '${rho}'
  []
  [forcing_u]
    type = ADParsedFunction
    value = 'mu*sin(y)*cos((1/2)*x*pi) + (1/4)*pi^2*mu*sin(y)*cos((1/2)*x*pi) - 1/2*pi*rho*sin(x)*sin(y)*sin((1/2)*y*pi)*cos((1/2)*x*pi) + rho*sin(x)*cos(y)*cos((1/2)*x*pi)*cos((1/2)*y*pi) - pi*rho*sin(y)^2*sin((1/2)*x*pi)*cos((1/2)*x*pi) + sin(y)*cos(x)'
    vars = 'mu rho'
    vals = '${mu} ${rho}'
  []
  [exact_v]
    type = ParsedFunction
    value = 'sin(x)*cos((1/2)*y*pi)'
  []
  [exact_rhov]
    type = ParsedFunction
    value = 'rho*sin(x)*cos((1/2)*y*pi)'
    vars = 'rho'
    vals = '${rho}'
  []
  [forcing_v]
    type = ADParsedFunction
    value = 'mu*sin(x)*cos((1/2)*y*pi) + (1/4)*pi^2*mu*sin(x)*cos((1/2)*y*pi) - pi*rho*sin(x)^2*sin((1/2)*y*pi)*cos((1/2)*y*pi) - 1/2*pi*rho*sin(x)*sin(y)*sin((1/2)*x*pi)*cos((1/2)*y*pi) + rho*sin(y)*cos(x)*cos((1/2)*x*pi)*cos((1/2)*y*pi) + sin(x)*cos(y)'
    vars = 'mu rho'
    vals = '${mu} ${rho}'
  []
  [exact_p]
    type = ParsedFunction
    value = 'sin(x)*sin(y)'
  []
  [forcing_p]
    type = ParsedFunction
    value = '-1/2*pi*rho*sin(x)*sin((1/2)*y*pi) - 1/2*pi*rho*sin(y)*sin((1/2)*x*pi)'
    vars = 'rho'
    vals = '${rho}'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      30                lu           NONZERO'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
  csv = true
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [./L2u]
    type = ElementL2Error
    variable = u
    function = exact_u
    outputs = 'console csv'
    execute_on = 'timestep_end'
  [../]
  [./L2v]
    variable = v
    function = exact_v
    type = ElementL2Error
    outputs = 'console csv'
    execute_on = 'timestep_end'
  [../]
  [./L2p]
    variable = pressure
    function = exact_p
    type = ElementL2Error
    outputs = 'console csv'
    execute_on = 'timestep_end'
  [../]
[]
