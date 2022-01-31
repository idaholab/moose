mu=1.1
rho=1.1
advected_interp_method='average'
velocity_interp_method='rc'

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 2
    ny = 2
  []
[]

[Problem]
  fv_bcs_integrity_check = true
  coord_type = 'RZ'
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

[AuxVariables]
  [bx_out]
    type = MooseVariableFVReal
  []
  [by_out]
    type = MooseVariableFVReal
  []
  [b2x_out]
    type = MooseVariableFVReal
  []
  [b2y_out]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [bx_out]
    type = ADFunctorElementalAux
    variable = bx_out
    functor = "bx"
    execute_on = "timestep_end"
  []
  [by_out]
    type = ADFunctorElementalAux
    variable = by_out
    functor = "by"
    execute_on = "timestep_end"
  []
  [b2x_out]
    type = ADFunctorElementalAux
    variable = b2x_out
    functor = "b2x"
    execute_on = "timestep_end"
  []
  [b2y_out]
    type = ADFunctorElementalAux
    variable = b2y_out
    functor = "b2y"
    execute_on = "timestep_end"
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
    boundary = 'bottom'
    variable = u
    function = 'exact_u'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'bottom'
    variable = v
    function = 'exact_v'
  []
  [no-slip-wall-u]
    type = INSFVNoSlipWallBC
    boundary = 'right'
    variable = u
    function = 'exact_u'
  []
  [no-slip-wall-v]
    type = INSFVNoSlipWallBC
    boundary = 'right'
    variable = v
    function = 'exact_v'
  []
  [outlet-p]
    type = INSFVOutletPressureBC
    boundary = 'top'
    variable = pressure
    function = 'exact_p'
  []
  [axis-u]
    type = INSFVSymmetryVelocityBC
    boundary = 'left'
    variable = u
    u = u
    v = v
    mu = ${mu}
    momentum_component = x
  []
  [axis-v]
    type = INSFVSymmetryVelocityBC
    boundary = 'left'
    variable = v
    u = u
    v = v
    mu = ${mu}
    momentum_component = y
  []
  [axis-p]
    type = INSFVSymmetryPressureBC
    boundary = 'left'
    variable = pressure
  []
[]

[Materials]
  [ins_fv]
    type = INSFVMaterial
    u = 'u'
    v = 'v'
    pressure = 'pressure'
    rho = ${rho}
  []
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    value = 'sin(x*pi)^2*sin((1/2)*y*pi)'
  []
  [exact_rhou]
    type = ParsedFunction
    value = 'rho*sin(x*pi)^2*sin((1/2)*y*pi)'
    vars = 'rho'
    vals = '${rho}'
  []
  [forcing_u]
    type = ADParsedFunction
    value = '(1/4)*pi^2*mu*sin(x*pi)^2*sin((1/2)*y*pi) - pi*sin(x*pi)*cos((1/2)*y*pi) + (4*x*pi*rho*sin(x*pi)^3*sin((1/2)*y*pi)^2*cos(x*pi) + rho*sin(x*pi)^4*sin((1/2)*y*pi)^2)/x + (-x*pi*rho*sin(x*pi)^2*sin((1/2)*y*pi)*sin(y*pi)*cos(x*pi) + (1/2)*x*pi*rho*sin(x*pi)^2*cos(x*pi)*cos((1/2)*y*pi)*cos(y*pi))/x - (-2*x*pi^2*mu*sin(x*pi)^2*sin((1/2)*y*pi) + 2*x*pi^2*mu*sin((1/2)*y*pi)*cos(x*pi)^2 + 2*pi*mu*sin(x*pi)*sin((1/2)*y*pi)*cos(x*pi))/x'
    vars = 'mu rho'
    vals = '${mu} ${rho}'
  []
  [exact_v]
    type = ParsedFunction
    value = 'cos(x*pi)*cos(y*pi)'
  []
  [exact_rhov]
    type = ParsedFunction
    value = 'rho*cos(x*pi)*cos(y*pi)'
    vars = 'rho'
    vals = '${rho}'
  []
  [forcing_v]
    type = ADParsedFunction
    value = 'pi^2*mu*cos(x*pi)*cos(y*pi) - 2*pi*rho*sin(y*pi)*cos(x*pi)^2*cos(y*pi) - 1/2*pi*sin((1/2)*y*pi)*cos(x*pi) - (-x*pi^2*mu*cos(x*pi)*cos(y*pi) - pi*mu*sin(x*pi)*cos(y*pi))/x + (-x*pi*rho*sin(x*pi)^3*sin((1/2)*y*pi)*cos(y*pi) + 2*x*pi*rho*sin(x*pi)*sin((1/2)*y*pi)*cos(x*pi)^2*cos(y*pi) + rho*sin(x*pi)^2*sin((1/2)*y*pi)*cos(x*pi)*cos(y*pi))/x'
    vars = 'mu rho'
    vals = '${mu} ${rho}'
  []
  [exact_p]
    type = ParsedFunction
    value = 'cos(x*pi)*cos((1/2)*y*pi)'
  []
  [forcing_p]
    type = ParsedFunction
    value = '-pi*rho*sin(y*pi)*cos(x*pi) + (2*x*pi*rho*sin(x*pi)*sin((1/2)*y*pi)*cos(x*pi) + rho*sin(x*pi)^2*sin((1/2)*y*pi))/x'
    vars = 'rho'
    vals = '${rho}'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      200                lu           NONZERO'
  line_search = 'none'
[]

[Outputs]
  exodus = true
  csv = true
  [dof]
    type = DOFMap
    execute_on = 'initial'
  []
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
    type = ElementL2Error
    variable = v
    function = exact_v
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
