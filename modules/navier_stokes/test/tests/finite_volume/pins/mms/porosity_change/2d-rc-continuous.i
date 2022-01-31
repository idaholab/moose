mu=1.1
rho=1.1
advected_interp_method='average'
velocity_interp_method='rc'

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 2
    ymin = -1
    ymax = 1
    nx = 8
    ny = 8
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = PINSFVRhieChowInterpolator
    u = u
    v = v
    porosity = porosity
    pressure = pressure
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
  [porosity]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
[]

[ICs]
  [porosity_continuous]
    type = FunctionIC
    variable = porosity
    function = smooth_jump
  []
[]

[GlobalParams]
  porosity = porosity
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
[]

[Functions]
  [smooth_jump]
    type = ParsedFunction
    value = '1 - 0.5 * 1 / (1 + exp(-30*(x-1))) - 0.01 * y'
  []

  # Output from compute-functions-2d.py
  [exact_u]
    type = ParsedFunction
    value = 'sin((1/2)*y*pi)*cos((1/2)*x*pi)'
  []
  [forcing_u]
    type = ADParsedFunction
    value = '15.0*mu*(-1/2*pi*sin((1/2)*x*pi)*sin((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) + 15.0*exp(30 - 30*x)*sin((1/2)*y*pi)*cos((1/2)*x*pi)/((exp(30 - 30*x) + 1)^2*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2))*exp(30 - 30*x)/(exp(30 - 30*x) + 1)^2 + 0.01*mu*((1/2)*pi*cos((1/2)*x*pi)*cos((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) + 0.01*sin((1/2)*y*pi)*cos((1/2)*x*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2) - mu*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))*(-1/4*pi^2*sin((1/2)*y*pi)*cos((1/2)*x*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) + 0.01*pi*cos((1/2)*x*pi)*cos((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2 + 0.0002*sin((1/2)*y*pi)*cos((1/2)*x*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^3) - mu*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))*(-1/4*pi^2*sin((1/2)*y*pi)*cos((1/2)*x*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) - 15.0*pi*exp(30 - 30*x)*sin((1/2)*x*pi)*sin((1/2)*y*pi)/((exp(30 - 30*x) + 1)^2*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2) - 450.0*exp(30 - 30*x)*sin((1/2)*y*pi)*cos((1/2)*x*pi)/((exp(30 - 30*x) + 1)^2*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2) + 900.0*exp(60 - 60*x)*sin((1/2)*y*pi)*cos((1/2)*x*pi)/((exp(30 - 30*x) + 1)^3*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2) + 450.0*exp(60 - 60*x)*sin((1/2)*y*pi)*cos((1/2)*x*pi)/((exp(30 - 30*x) + 1)^4*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^3)) - 1/2*pi*rho*sin((1/4)*x*pi)*sin((1/2)*y*pi)^2*cos((1/2)*x*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) + (1/2)*pi*rho*sin((1/4)*x*pi)*cos((1/2)*x*pi)*cos((1/2)*y*pi)^2/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) - pi*rho*sin((1/2)*x*pi)*sin((1/2)*y*pi)^2*cos((1/2)*x*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) + 0.01*rho*sin((1/4)*x*pi)*sin((1/2)*y*pi)*cos((1/2)*x*pi)*cos((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2 + 15.0*rho*exp(30 - 30*x)*sin((1/2)*y*pi)^2*cos((1/2)*x*pi)^2/((exp(30 - 30*x) + 1)^2*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2) - 1/4*pi*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))*sin((1/4)*x*pi)*sin((3/2)*y*pi)'
    vars = 'mu rho'
    vals = '${mu} ${rho}'
  []
  [exact_v]
    type = ParsedFunction
    value = 'sin((1/4)*x*pi)*cos((1/2)*y*pi)'
  []
  [forcing_v]
    type = ADParsedFunction
    value = '0.01*mu*(-1/2*pi*sin((1/4)*x*pi)*sin((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) + 0.01*sin((1/4)*x*pi)*cos((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2) + 15.0*mu*((1/4)*pi*cos((1/4)*x*pi)*cos((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) + 15.0*exp(30 - 30*x)*sin((1/4)*x*pi)*cos((1/2)*y*pi)/((exp(30 - 30*x) + 1)^2*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2))*exp(30 - 30*x)/(exp(30 - 30*x) + 1)^2 - mu*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))*(-1/4*pi^2*sin((1/4)*x*pi)*cos((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) - 0.01*pi*sin((1/4)*x*pi)*sin((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2 + 0.0002*sin((1/4)*x*pi)*cos((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^3) - mu*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))*(-1/16*pi^2*sin((1/4)*x*pi)*cos((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) - 450.0*exp(30 - 30*x)*sin((1/4)*x*pi)*cos((1/2)*y*pi)/((exp(30 - 30*x) + 1)^2*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2) + 7.5*pi*exp(30 - 30*x)*cos((1/4)*x*pi)*cos((1/2)*y*pi)/((exp(30 - 30*x) + 1)^2*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2) + 900.0*exp(60 - 60*x)*sin((1/4)*x*pi)*cos((1/2)*y*pi)/((exp(30 - 30*x) + 1)^3*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2) + 450.0*exp(60 - 60*x)*sin((1/4)*x*pi)*cos((1/2)*y*pi)/((exp(30 - 30*x) + 1)^4*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^3)) - pi*rho*sin((1/4)*x*pi)^2*sin((1/2)*y*pi)*cos((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) - 1/2*pi*rho*sin((1/4)*x*pi)*sin((1/2)*x*pi)*sin((1/2)*y*pi)*cos((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) + (1/4)*pi*rho*sin((1/2)*y*pi)*cos((1/4)*x*pi)*cos((1/2)*x*pi)*cos((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) + 0.01*rho*sin((1/4)*x*pi)^2*cos((1/2)*y*pi)^2/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2 + 15.0*rho*exp(30 - 30*x)*sin((1/4)*x*pi)*sin((1/2)*y*pi)*cos((1/2)*x*pi)*cos((1/2)*y*pi)/((exp(30 - 30*x) + 1)^2*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2) + (3/2)*pi*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))*cos((1/4)*x*pi)*cos((3/2)*y*pi)'
    vars = 'mu rho'
    vals = '${mu} ${rho}'
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
  exodus = true
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
