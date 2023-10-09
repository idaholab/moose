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
    pressure = pressure
    disp_x = disp_x
    use_displaced_mesh = true
  []
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = -1
    xmax = 1
    nx = 2
  []
  displacements = 'disp_x'
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
  []
[]

[AuxVariables]
  [disp_x][]
  [pressure]
    type = INSFVPressureVariable
  []
[]

[ICs]
  [pressure]
    type = FunctionIC
    function = 'x^3'
    variable = pressure
  []
[]

[AuxKernels]
  [disp_x]
    type = FunctionAux
    function = exact_disp_x
    variable = disp_x
    execute_on = 'initial timestep_begin'
  []
[]

[FVKernels]
  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    advected_interp_method = 'average'
    rho = ${rho}
    momentum_component = 'x'
    use_displaced_mesh = true
    boundaries_to_force = 'left right'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = u
    mu = ${mu}
    momentum_component = 'x'
    use_displaced_mesh = true
  []
  [u_mesh_advection]
    type = INSFVMomentumMeshAdvection
    variable = u
    rho = ${rho}
    momentum_component = 'x'
    disp_x = disp_x
    use_displaced_mesh = true
  []
  [u_forcing]
    type = INSFVBodyForce
    variable = u
    functor = forcing_u
    momentum_component = 'x'
    use_displaced_mesh = true
  []
[]

[FVBCs]
  [no-slip-wall-u]
    type = INSFVNoSlipWallBC
    boundary = 'left right'
    variable = u
    function = 'exact_u'
  []
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    expression = 'cos(x)'
  []
  [forcing_u]
    type = ParsedFunction
    expression = 'mu*cos(x) - rho*(-2*x/(2*t + 1) + cos(x))*sin(x) + rho*(-sin(x) - 2/(2*t + 1))*cos(x) + 2*rho*cos(x)/(2*t + 1)'
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
  []
  [exact_disp_x]
    type = ParsedFunction
    expression = '2*x*t'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  dt = 0.5
  nl_rel_tol = 1e-12
[]

[Outputs]
  csv = true
  exodus = true
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
    use_displaced_mesh = true
  []
  [L2u]
    type = ElementL2FunctorError
    approximate = u
    exact = exact_u
    outputs = 'console csv'
    execute_on = 'timestep_end'
    use_displaced_mesh = true
  []
[]
