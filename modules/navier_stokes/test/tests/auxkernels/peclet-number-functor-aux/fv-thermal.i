mu = 1
rho = 1
k = 1
cp = 1

[GlobalParams]
  velocity_interp_method = 'rc'
  # Maximum cell Peclet number is ~.1 so energy transport is stable without upwinding
  advected_interp_method = 'average'
  rhie_chow_user_object = 'rc'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
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
  [T]
    type = INSFVEnergyVariable
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]

[AuxVariables]
  [Pe]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [Pe]
    type = PecletNumberFunctorAux
    variable = Pe
    speed = speed
    thermal_diffusivity = 'thermal_diffusivity'
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

  [temp_conduction]
    type = FVDiffusion
    coeff = ${k}
    variable = T
  []
  [temp_advection]
    type = INSFVEnergyAdvection
    variable = T
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

  [T_hot]
    type = FVDirichletBC
    variable = T
    boundary = 'bottom'
    value = 1
  []
  [T_cold]
    type = FVDirichletBC
    variable = T
    boundary = 'top'
    value = 0
  []
[]

[Materials]
  [mu]
    type = ADGenericFunctorMaterial
    prop_names = 'mu'
    prop_values = '${mu}'
  []
  [speed]
    type = ADVectorMagnitudeFunctorMaterial
    x_functor = u
    y_functor = v
    vector_magnitude_name = speed
  []
  [thermal_diffusivity]
    type = ThermalDiffusivityFunctorMaterial
    k = ${k}
    rho = ${rho}
    cp = ${cp}
  []
  [enthalpy]
    type = INSFVEnthalpyMaterial
    rho = ${rho}
    temperature = T
    cp = ${cp}
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      100                lu           NONZERO'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
