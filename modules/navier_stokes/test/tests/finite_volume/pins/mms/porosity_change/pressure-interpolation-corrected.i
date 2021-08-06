mu=1.1
rho=1.1
advected_interp_method='average'
velocity_interp_method='rc'
darcy=1.1
forch=1.1

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
  Darcy_name = 'Darcy_coefficient'
  Forchheimer_name = 'Forchheimer_coefficient'
  porosity = porosity
[]

[UserObjects]
  [rc]
    type = PINSFVRhieChowInterpolator
    u = u
    v = v
    porosity = porosity
    pressure = pressure
    smoothing_layers = 2
  []
[]

[Problem]
  fv_bcs_integrity_check = true
[]

[Variables]
  [u]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 1
  []
  [v]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 1
  []
  [pressure]
    type = INSFVPressureVariable
  []
[]

[AuxVariables]
  [eps_out]
    type = MooseVariableFVReal
  []
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
  [eps_out]
    type = ADFunctorElementalAux
    variable = eps_out
    functor = porosity
    execute_on = 'timestep_end'
  []
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
    type = PINSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    u = u
    v = v
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
    u = u
    v = v
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = PINSFVMomentumDiffusion
    variable = u
    mu = ${mu}
    porosity = porosity
    momentum_component = 'x'
  []
  [u_pressure]
    type = PINSFVMomentumPressure
    variable = u
    pressure = pressure
    porosity = porosity
    momentum_component = 'x'
  []
  [u_drag]
    type = PINSFVMomentumFriction
    variable = u
    momentum_component = 'x'
    porosity = porosity
    rho = ${rho}
  []
  [u_correction]
    type = PINSFVMomentumFrictionCorrection
    variable = u
    momentum_component = 'x'
    porosity = porosity
    rho = ${rho}
  []
  [u_forcing]
    type = INSFVBodyForce
    variable = u
    functor = forcing_u
    momentum_component = 'x'
  []

  [v_advection]
    type = PINSFVMomentumAdvection
    variable = v
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    u = u
    v = v
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = PINSFVMomentumDiffusion
    variable = v
    mu = ${mu}
    porosity = porosity
    momentum_component = 'y'
  []
  [v_pressure]
    type = PINSFVMomentumPressure
    variable = v
    pressure = pressure
    porosity = porosity
    momentum_component = 'y'
  []
  [v_drag]
    type = PINSFVMomentumFriction
    variable = v
    momentum_component = 'y'
    porosity = porosity
    rho = ${rho}
  []
  [v_correction]
    type = PINSFVMomentumFrictionCorrection
    variable = v
    momentum_component = 'y'
    porosity = porosity
    rho = ${rho}
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

[Materials]
  [ins_fv]
    type = INSFVMaterial
    u = 'u'
    v = 'v'
    pressure = 'pressure'
    rho = ${rho}
  []
  [darcy]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'Darcy_coefficient Forchheimer_coefficient'
    prop_values = '${darcy} ${darcy} ${darcy} ${forch} ${forch} ${forch}'
  []
[]

[Functions]
  [porosity]
    type = ADParsedFunction
    value = '.5 + .1 * sin(pi * x / 4) * cos(pi * y / 4)'
  []

  [exact_u]
    type = ParsedFunction
    value = 'sin((1/2)*y*pi)*cos((1/2)*x*pi)'
  []
  [forcing_u]
    type = ADParsedFunction
    value = '-mu*(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)*(-1/4*pi^2*sin((1/2)*y*pi)*cos((1/2)*x*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) + 0.025*pi^2*sin((1/4)*x*pi)*sin((1/4)*y*pi)*cos((1/2)*x*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 + 0.00625*pi^2*sin((1/4)*x*pi)*sin((1/2)*y*pi)*cos((1/2)*x*pi)*cos((1/4)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 + 0.00125*pi^2*sin((1/4)*x*pi)^2*sin((1/4)*y*pi)^2*sin((1/2)*y*pi)*cos((1/2)*x*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^3) - mu*(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)*(-1/4*pi^2*sin((1/2)*y*pi)*cos((1/2)*x*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) + 0.00625*pi^2*sin((1/4)*x*pi)*sin((1/2)*y*pi)*cos((1/2)*x*pi)*cos((1/4)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 + 0.025*pi^2*sin((1/2)*x*pi)*sin((1/2)*y*pi)*cos((1/4)*x*pi)*cos((1/4)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 + 0.00125*pi^2*sin((1/2)*y*pi)*cos((1/4)*x*pi)^2*cos((1/2)*x*pi)*cos((1/4)*y*pi)^2/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^3) - 0.025*pi*mu*(-1/2*pi*sin((1/2)*x*pi)*sin((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) - 0.025*pi*sin((1/2)*y*pi)*cos((1/4)*x*pi)*cos((1/2)*x*pi)*cos((1/4)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2)*cos((1/4)*x*pi)*cos((1/4)*y*pi) + 0.025*pi*mu*((1/2)*pi*cos((1/2)*x*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) + 0.025*pi*sin((1/4)*x*pi)*sin((1/4)*y*pi)*sin((1/2)*y*pi)*cos((1/2)*x*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2)*sin((1/4)*x*pi)*sin((1/4)*y*pi) + rho*(darcy + forch)*sin((1/2)*y*pi)*cos((1/2)*x*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) - 1/2*pi*rho*sin((1/4)*x*pi)*sin((1/2)*y*pi)^2*cos((1/2)*x*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) + (1/2)*pi*rho*sin((1/4)*x*pi)*cos((1/2)*x*pi)*cos((1/2)*y*pi)^2/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) - pi*rho*sin((1/2)*x*pi)*sin((1/2)*y*pi)^2*cos((1/2)*x*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) + 0.025*pi*rho*sin((1/4)*x*pi)^2*sin((1/4)*y*pi)*sin((1/2)*y*pi)*cos((1/2)*x*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 - 0.025*pi*rho*sin((1/2)*y*pi)^2*cos((1/4)*x*pi)*cos((1/2)*x*pi)^2*cos((1/4)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 - 1/4*pi*(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)*sin((1/4)*x*pi)*sin((3/2)*y*pi)'
    vars = 'mu rho darcy forch'
    vals = '${mu} ${rho} ${darcy} ${forch}'
  []
  [exact_v]
    type = ParsedFunction
    value = 'sin((1/4)*x*pi)*cos((1/2)*y*pi)'
  []
  [forcing_v]
    type = ADParsedFunction
    value = '-mu*(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)*(-1/4*pi^2*sin((1/4)*x*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) - 0.025*pi^2*sin((1/4)*x*pi)^2*sin((1/4)*y*pi)*sin((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 + 0.00625*pi^2*sin((1/4)*x*pi)^2*cos((1/4)*y*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 + 0.00125*pi^2*sin((1/4)*x*pi)^3*sin((1/4)*y*pi)^2*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^3) - mu*(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)*(-1/16*pi^2*sin((1/4)*x*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) + 0.00625*pi^2*sin((1/4)*x*pi)^2*cos((1/4)*y*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 - 0.0125*pi^2*cos((1/4)*x*pi)^2*cos((1/4)*y*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 + 0.00125*pi^2*sin((1/4)*x*pi)*cos((1/4)*x*pi)^2*cos((1/4)*y*pi)^2*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^3) + 0.025*pi*mu*(-1/2*pi*sin((1/4)*x*pi)*sin((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) + 0.025*pi*sin((1/4)*x*pi)^2*sin((1/4)*y*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2)*sin((1/4)*x*pi)*sin((1/4)*y*pi) - 0.025*pi*mu*((1/4)*pi*cos((1/4)*x*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) - 0.025*pi*sin((1/4)*x*pi)*cos((1/4)*x*pi)*cos((1/4)*y*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2)*cos((1/4)*x*pi)*cos((1/4)*y*pi) + rho*(darcy + forch)*sin((1/4)*x*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) - pi*rho*sin((1/4)*x*pi)^2*sin((1/2)*y*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) - 1/2*pi*rho*sin((1/4)*x*pi)*sin((1/2)*x*pi)*sin((1/2)*y*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) + (1/4)*pi*rho*sin((1/2)*y*pi)*cos((1/4)*x*pi)*cos((1/2)*x*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) + 0.025*pi*rho*sin((1/4)*x*pi)^3*sin((1/4)*y*pi)*cos((1/2)*y*pi)^2/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 - 0.025*pi*rho*sin((1/4)*x*pi)*sin((1/2)*y*pi)*cos((1/4)*x*pi)*cos((1/2)*x*pi)*cos((1/4)*y*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 + (3/2)*pi*(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)*cos((1/4)*x*pi)*cos((3/2)*y*pi)'
    vars = 'mu rho darcy forch'
    vals = '${mu} ${rho} ${darcy} ${forch}'
  []
  [exact_p]
    type = ParsedFunction
    value = 'sin((3/2)*y*pi)*cos((1/4)*x*pi)'
  []
  [forcing_p]
    type = ParsedFunction
    value = '-1/2*pi*rho*sin((1/4)*x*pi)*sin((1/2)*y*pi) - 1/2*pi*rho*sin((1/2)*x*pi)*sin((1/2)*y*pi)'
    vars = 'rho'
    vals = '${rho}'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      100                lu           NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = false
  csv = true
[]

[Postprocessors]
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
  [L2v]
    type = ElementL2Error
    variable = v
    function = exact_v
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2p]
    type = ElementL2Error
    variable = pressure
    function = exact_p
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]
