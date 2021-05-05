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
  two_term_boundary_expansion = true
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
  inactive = 'u_pressure_porosity u_pressure_porosity_gradient
              v_pressure_porosity v_pressure_porosity_gradient'
  [mass]
    type = PINSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    vel = 'velocity'
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    smooth_porosity = true
  []
  [mass_forcing]
    type = FVBodyForce
    variable = pressure
    function = forcing_p
  []

  [u_advection]
    type = PINSFVMomentumAdvection
    variable = u
    advected_quantity = 'rhou'
    vel = 'velocity'
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    smooth_porosity = true
  []
  [u_viscosity]
    type = PINSFVMomentumDiffusion
    variable = u
    mu = ${mu}
    porosity = porosity
    smooth_porosity = false
    vel = 'velocity'
    momentum_component = 'x'
  []
  # Option 1: eps * pressure gradient
  [u_pressure]
    type = PINSFVMomentumPressure
    variable = u
    p = pressure
    porosity = porosity
    momentum_component = 'x'
  []

  # Option 2: gradient (eps * pressure) - P * gradient(eps)
  [u_pressure_porosity]
    type = PINSFVMomentumPressureFlux
    variable = u
    p = pressure
    porosity = porosity
    momentum_component = 'x'
  []
  [u_pressure_porosity_gradient]
    type = PINSFVMomentumPressurePorosityGradient
    variable = u
    p = pressure
    porosity = porosity
    momentum_component = 'x'
  []
  [u_forcing]
    type = FVBodyForce
    variable = u
    function = forcing_u
  []

  [v_advection]
    type = PINSFVMomentumAdvection
    variable = v
    advected_quantity = 'rhov'
    vel = 'velocity'
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    smooth_porosity = true
  []
  [v_viscosity]
    type = PINSFVMomentumDiffusion
    variable = v
    mu = ${mu}
    porosity = porosity
    smooth_porosity = false
    vel = 'velocity'
    momentum_component = 'y'
  []
  # Option 1: eps * pressure gradient
  [v_pressure]
    type = PINSFVMomentumPressure
    variable = v
    p = pressure
    porosity = porosity
    momentum_component = 'y'
  []

  # Option 2: gradient (eps * pressure) - P * gradient(eps)
  [v_pressure_porosity]
    type = PINSFVMomentumPressureFlux
    variable = v
    p = pressure
    porosity = porosity
    momentum_component = 'y'
  []
  [v_pressure_porosity_gradient]
    type = PINSFVMomentumPressurePorosityGradient
    variable = v
    p = pressure
    porosity = porosity
    momentum_component = 'y'
  []
  [v_forcing]
    type = FVBodyForce
    variable = v
    function = forcing_v
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
    type = ParsedFunction
    value = '(1/2)*pi^2*mu*sin((1/2)*y*pi)*cos((1/2)*x*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) - 0.005*pi*mu*cos((1/2)*x*pi)*cos((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2 + 7.5*pi*mu*exp(30 - 30*x)*sin((1/2)*x*pi)*sin((1/2)*y*pi)/((exp(30 - 30*x) + 1)^2*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2) - 1/2*pi*rho*sin((1/4)*x*pi)*sin((1/2)*y*pi)^2*cos((1/2)*x*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) + (1/2)*pi*rho*sin((1/4)*x*pi)*cos((1/2)*x*pi)*cos((1/2)*y*pi)^2/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) - pi*rho*sin((1/2)*x*pi)*sin((1/2)*y*pi)^2*cos((1/2)*x*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) + 0.01*rho*sin((1/4)*x*pi)*sin((1/2)*y*pi)*cos((1/2)*x*pi)*cos((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2 + 15.0*rho*exp(30 - 30*x)*sin((1/2)*y*pi)^2*cos((1/2)*x*pi)^2/((exp(30 - 30*x) + 1)^2*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2) - 1/4*pi*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))*sin((1/4)*x*pi)*sin((3/2)*y*pi)'
    vars = 'mu rho'
    vals = '${mu} ${rho}'
  []
  [exact_v]
    type = ParsedFunction
    value = 'sin((1/4)*x*pi)*cos((1/2)*y*pi)'
  []
  [forcing_v]
    type = ParsedFunction
    value = '(5/16)*pi^2*mu*sin((1/4)*x*pi)*cos((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) + 0.005*pi*mu*sin((1/4)*x*pi)*sin((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2 - 3.75*pi*mu*exp(30 - 30*x)*cos((1/4)*x*pi)*cos((1/2)*y*pi)/((exp(30 - 30*x) + 1)^2*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2) - pi*rho*sin((1/4)*x*pi)^2*sin((1/2)*y*pi)*cos((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) - 1/2*pi*rho*sin((1/4)*x*pi)*sin((1/2)*x*pi)*sin((1/2)*y*pi)*cos((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) + (1/4)*pi*rho*sin((1/2)*y*pi)*cos((1/4)*x*pi)*cos((1/2)*x*pi)*cos((1/2)*y*pi)/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1)) + 0.01*rho*sin((1/4)*x*pi)^2*cos((1/2)*y*pi)^2/(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2 + 15.0*rho*exp(30 - 30*x)*sin((1/4)*x*pi)*sin((1/2)*y*pi)*cos((1/2)*x*pi)*cos((1/2)*y*pi)/((exp(30 - 30*x) + 1)^2*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))^2) + (3/2)*pi*(-0.01*y + 1 - 0.5/(exp(30 - 30*x) + 1))*cos((1/4)*x*pi)*cos((3/2)*y*pi)'
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
