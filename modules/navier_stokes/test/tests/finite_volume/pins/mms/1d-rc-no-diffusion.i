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
    value = 'cos((1/2)*x*pi)'
  []
  [forcing_u]
    type = ADParsedFunction
    value = '-1.25*pi*rho*sin((1/2)*x*pi)*cos((1/2)*x*pi) + 0.8*cos(x)'
    vars = 'mu rho'
    vals = '${mu} ${rho}'
  []
  [exact_p]
    type = ParsedFunction
    value = 'sin(x)'
  []
  [forcing_p]
    type = ParsedFunction
    value = '-1/2*pi*rho*sin((1/2)*x*pi)'
    vars = 'rho'
    vals = '${rho}'
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
  []
  [u_forcing]
    type = INSFVBodyForce
    variable = u
    functor = forcing_u
    momentum_component = 'x'
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = u
    function = 'exact_u'
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = 'exact_p'
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
  [inlet_p]
    type = SideAverageValue
    variable = 'pressure'
    boundary = 'left'
  []
  [outlet-u]
    type = SideIntegralVariablePostprocessor
    variable = u
    boundary = 'right'
  []
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2u]
    type = ElementL2Error
    variable = u
    function = exact_u
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2p]
    variable = pressure
    function = exact_p
    type = ElementL2Error
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
