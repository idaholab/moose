mu=1.1
rho=1.1
k=1.1
cp=1.1
advected_interp_method='average'
velocity_interp_method='average'

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
    standard_body_forces = true
    pressure = pressure
  []
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
    initial_condition = 1
    two_term_boundary_expansion = false
  []
  [v]
    type = INSFVVelocityVariable
    initial_condition = 1
    two_term_boundary_expansion = false
  []
  [pressure]
    type = INSFVPressureVariable
    two_term_boundary_expansion = false
  []
  [temperature]
    type = INSFVEnergyVariable
    two_term_boundary_expansion = false
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

  [temp_conduction]
    type = FVDiffusion
    coeff = 'k'
    variable = temperature
  []
  [temp_advection]
    type = INSFVEnergyAdvection
    variable = temperature
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
  []
  [temp_forcing]
    type = FVBodyForce
    variable = temperature
    function = forcing_t
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
  [axis-inlet-wall-t]
    type = FVFunctionDirichletBC
    boundary = 'left bottom right'
    variable = temperature
    function = 'exact_t'
  []
[]

[Materials]
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'cp k'
    prop_values = '${cp} ${k}'
  []
  [ins_fv]
    type = INSFVEnthalpyMaterial
    temperature = 'temperature'
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
  [exact_t]
    type = ParsedFunction
    value = 'sin(x*pi)*sin((1/2)*y*pi)'
  []
  [forcing_t]
    type = ParsedFunction
    value = '(1/4)*pi^2*k*sin(x*pi)*sin((1/2)*y*pi) - (-x*pi^2*k*sin(x*pi)*sin((1/2)*y*pi) + pi*k*sin((1/2)*y*pi)*cos(x*pi))/x + (3*x*pi*cp*rho*sin(x*pi)^2*sin((1/2)*y*pi)^2*cos(x*pi) + cp*rho*sin(x*pi)^3*sin((1/2)*y*pi)^2)/x + (-x*pi*cp*rho*sin(x*pi)*sin((1/2)*y*pi)*sin(y*pi)*cos(x*pi) + (1/2)*x*pi*cp*rho*sin(x*pi)*cos(x*pi)*cos((1/2)*y*pi)*cos(y*pi))/x'
    vars = 'k rho cp'
    vals = '${k} ${rho} ${cp}'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      100                lu           NONZERO'
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
  [./L2t]
    variable = temperature
    function = exact_t
    type = ElementL2Error
    outputs = 'console csv'
    execute_on = 'timestep_end'
  [../]
[]
