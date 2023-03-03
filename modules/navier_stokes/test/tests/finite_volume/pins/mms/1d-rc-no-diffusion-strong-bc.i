mu=1e-15
rho=1.1
advected_interp_method='upwind'
velocity_interp_method='rc'

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
    xmax = 0.5
  []
[]

[GlobalParams]
  two_term_boundary_expansion = true
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = PINSFVRhieChowInterpolator
    u = u
    pressure = pressure
    porosity = porosity
  []
[]

[Problem]
  fv_bcs_integrity_check = false
[]

[Variables]
  [u]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = .1
  []
  [pressure]
    type = INSFVPressureVariable
  []
[]

[AuxVariables]
  [porosity]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 0.8
  []
[]

[Problem]
  error_on_jacobian_nonzero_reallocation = true
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    expression = 'cos((1/2)*x*pi)'
  []
  [forcing_u]
    type = ParsedFunction
    expression = '-1.25*pi*rho*sin((1/2)*x*pi)*cos((1/2)*x*pi) + 0.8*cos(x)'
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
  []
  [exact_p]
    type = ParsedFunction
    expression = 'sin(x)'
  []
  [forcing_p]
    type = ParsedFunction
    expression = '-1/2*pi*rho*sin((1/2)*x*pi)'
    symbol_names = 'rho'
    symbol_values = '${rho}'
  []
[]

[FVKernels]
  [mass]
    type = PINSFVMassAdvection
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
    type = PINSFVMomentumAdvection
    variable = u
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    porosity = porosity
    momentum_component = 'x'
  []
  [u_pressure]
    type = PINSFVMomentumPressureFlux
    variable = u
    pressure = pressure
    porosity = porosity
    momentum_component = 'x'
    force_boundary_execution = false
  []
  [u_forcing]
    type = INSFVBodyForce
    variable = u
    functor = forcing_u
    momentum_component = 'x'
  []
[]

[FVBCs]
  [mass]
    variable = pressure
    type = PINSFVFunctorBC
    boundary = 'left right'
    superficial_vel_x = u
    pressure = pressure
    eqn = 'mass'
    porosity = porosity
  []
  [momentum]
    variable = u
    type = PINSFVFunctorBC
    boundary = 'left right'
    superficial_vel_x = u
    pressure = pressure
    eqn = 'momentum'
    momentum_component = 'x'
    porosity = porosity
  []

  [inlet-u]
    type = FVFunctionDirichletBC
    boundary = 'left'
    variable = u
    function = 'exact_u'
  []
  [outlet_p]
    type = FVFunctionDirichletBC
    boundary = 'right'
    variable = pressure
    function = 'exact_p'
  []
[]

[Materials]
  [const]
    type = ADGenericFunctorMaterial
    prop_names = 'rho'
    prop_values = '${rho}'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      100                lu           NONZERO'
  line_search = 'bt'
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
  [L2p]
    approximate = pressure
    exact = exact_p
    type = ElementL2FunctorError
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  csv = true
[]
