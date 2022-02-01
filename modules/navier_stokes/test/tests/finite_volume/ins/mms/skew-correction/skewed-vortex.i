mu=1.0
rho=1.0

[Problem]
  coord_type = 'XYZ'
  error_on_jacobian_nonzero_reallocation = true
[]

[Mesh]
  [gen_mesh]
    type = FileMeshGenerator
    file = skewed.msh
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 1
    face_interp_method = 'skewness-corrected'
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 1
    face_interp_method = 'skewness-corrected'
  []
  [pressure]
    type = INSFVPressureVariable
    face_interp_method = 'skewness-corrected'
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
    advected_interp_method = 'skewness-corrected'
    velocity_interp_method = 'rc'
    rho = ${rho}
  []
  [mean_zero_pressure]
    type = FVScalarLagrangeMultiplier
    variable = pressure
    lambda = lambda
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    advected_interp_method = 'skewness-corrected'
    velocity_interp_method = 'rc'
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
  []
  [u_forcing]
    type = INSFVBodyForce
    variable = vel_x
    functor = forcing_u
    momentum_component = 'x'
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    advected_interp_method = 'skewness-corrected'
    velocity_interp_method = 'rc'
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
  []
  [v_forcing]
    type = INSFVBodyForce
    variable = vel_y
    functor = forcing_v
    momentum_component = 'y'
  []
[]

[FVBCs]
  [no-slip-wall-u]
    type = INSFVNoSlipWallBC
    boundary = 'left right top bottom'
    variable = vel_x
    function = '0'
  []
  [no-slip-wall-v]
    type = INSFVNoSlipWallBC
    boundary = 'left right top bottom'
    variable = vel_y
    function = '0'
  []
[]

[Materials]
  [ins_fv]
    type = INSFVMaterial
    u = vel_x
    v = vel_y
    pressure = 'pressure'
    rho = ${rho}
  []
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    value = 'x^2*(1-x)^2*(2*y-6*y^2+4*y^3)'
  []
  [exact_v]
    type = ParsedFunction
    value = '-y^2*(1-y)^2*(2*x-6*x^2+4*x^3)'
  []
  [exact_p]
    type = ParsedFunction
    value = 'x*(1-x)-2/12'
  []
  [forcing_u]
    type = ADParsedFunction
    value = '-4*mu/rho*(-1+2*y)*(y^2-6*x*y^2+6*x^2*y^2-y+6*x*y-6*x^2*y+3*x^2-6*x^3+3*x^4)+1-2*x+4*x^3*y^2*(2*y^2-2*y+1)*(y-1)^2*(-1+2*x)*(x-1)^3'
    vars = 'mu rho'
    vals = '${mu} ${rho}'
  []
  [forcing_v]
    type = ADParsedFunction
    value = '4*mu/rho*(-1+2*x)*(x^2-6*y*x^2+6*x^2*y^2-x+6*x*y-6*x*y^2+3*y^2-6*y^3+3*y^4)+4*y^3*x^2*(2*x^2-2*x+1)*(x-1)^2*(-1+2*y)*(y-1)^3'
    vars = 'mu rho'
    vals = '${mu} ${rho}'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'bjacobi      30                 lu           NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-8
[]

[Outputs]
  [out]
    type = Exodus
    hide = lambda
  []
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
    variable = vel_x
    function = exact_u
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2v]
    type = ElementL2Error
    variable = vel_y
    function = exact_v
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
