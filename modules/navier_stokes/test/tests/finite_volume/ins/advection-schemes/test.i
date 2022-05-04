mu=10
rho=1

[GlobalParams]
  velocity_interp_method = 'rc'
  advected_interp_method = 'sou'
  rhie_chow_user_object = 'rc'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 11
    ny = 11
  []
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
  []
  [v]
    type = INSFVVelocityVariable
  []
  [pressure]
    type = INSFVPressureVariable
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = u
    v = v
    pressure = pressure
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    rho = ${rho}
  []
  [mean_zero_pressure]
    type = FVIntegralValueConstraint
    variable = pressure
    lambda = lambda
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    rho = ${rho}
    momentum_component = 'x'
  []

  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = u
    mu = 'mu'
    momentum_component = 'x'
  []

  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    pressure = pressure
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = v
    rho = ${rho}
    momentum_component = 'y'
  []

  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = v
    mu = 'mu'
    momentum_component = 'y'
  []

  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    pressure = pressure
  []
[]

[FVBCs]
  [top_x]
    type = INSFVNoSlipWallBC
    variable = u
    boundary = 'top'
    function = 1
  []

  [no_slip_x]
    type = INSFVNoSlipWallBC
    variable = u
    boundary = 'left right bottom'
    function = 0
  []

  [no_slip_y]
    type = INSFVNoSlipWallBC
    variable = v
    boundary = 'left right top bottom'
    function = 0
  []
[]

[Materials]
  [mu]
    type = ADGenericFunctorMaterial
    prop_names = 'mu'
    prop_values = '${mu}'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
