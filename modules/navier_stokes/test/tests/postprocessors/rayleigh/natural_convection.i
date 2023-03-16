mu = 1
rho = 1.1
beta = 1e-4
k = .01
cp = 1000
velocity_interp_method = 'rc'
advected_interp_method = 'average'
l = 4

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = ${l}
    nx = 8
    ny = 8
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
  [mass_time]
    type = WCNSFVMassTimeDerivative
    variable = pressure
    drho_dt = drho_dt
  []
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    rhie_chow_user_object = 'rc'
  []

  [u_time]
    type = WCNSFVMomentumTimeDerivative
    variable = u
    drho_dt = drho_dt
    rho = rho
    momentum_component = 'x'
    rhie_chow_user_object = 'rc'
  []
  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    rho = ${rho}
    momentum_component = 'x'
    rhie_chow_user_object = 'rc'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = u
    mu = ${mu}
    momentum_component = 'x'
    rhie_chow_user_object = 'rc'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    pressure = pressure
    rhie_chow_user_object = 'rc'
  []

  [v_time]
    type = WCNSFVMomentumTimeDerivative
    variable = v
    drho_dt = drho_dt
    rho = rho
    momentum_component = 'y'
    rhie_chow_user_object = 'rc'
  []
  [v_advection]
    type = INSFVMomentumAdvection
    variable = v
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    rho = ${rho}
    momentum_component = 'y'
    rhie_chow_user_object = 'rc'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = v
    mu = ${mu}
    momentum_component = 'y'
    rhie_chow_user_object = 'rc'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    pressure = pressure
    rhie_chow_user_object = 'rc'
  []

  [temp_time]
    type = WCNSFVEnergyTimeDerivative
    variable = T
    cp = cp
    rho = rho
    drho_dt = drho_dt
  []
  [temp_conduction]
    type = FVDiffusion
    coeff = 'k'
    variable = T
  []
  [temp_advection]
    type = INSFVEnergyAdvection
    variable = T
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    rhie_chow_user_object = 'rc'
  []
[]

[FVBCs]
  [no_slip_x]
    type = INSFVNoSlipWallBC
    variable = u
    boundary = 'left right bottom top'
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

[FluidProperties]
  [fp]
    type = SimpleFluidProperties
    density0 = ${rho}
    thermal_expansion = ${beta}
  []
[]

[Materials]
  [rho]
    type = RhoFromPTFunctorMaterial
    fp = fp
    temperature = T
    pressure = pressure
  []
  [functor_constants]
    type = ADGenericFunctorMaterial
    prop_names = 'cp k'
    prop_values = '${cp} ${k}'
  []
  [ins_fv]
    type = INSFVEnthalpyMaterial
    temperature = 'T'
    rho = ${rho}
  []
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 10
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      300                lu           NONZERO'
  nl_abs_tol = 1e-11

  automatic_scaling = true
[]

[Postprocessors]
  [rayleigh_1]
    type = RayleighNumber
    rho_min = rho_min
    rho_max = rho_max
    rho_ave = ${rho}
    l = ${l}
    mu_ave = ${mu}
    k_ave = ${k}
    cp_ave = ${cp}
    gravity_magnitude = 9.81
  []
  [rayleigh_2]
    type = RayleighNumber
    T_cold = T_min
    T_hot = T_max
    rho_ave = ${rho}
    beta = ${beta}
    l = ${l}
    mu_ave = ${mu}
    k_ave = ${k}
    cp_ave = ${cp}
    gravity_magnitude = 9.81
  []

  [rho_min]
    type = ADElementExtremeFunctorValue
    functor = 'rho'
    value_type = 'min'
  []
  [rho_max]
    type = ADElementExtremeFunctorValue
    functor = 'rho'
    value_type = 'max'
  []
  [T_min]
    type = ADElementExtremeFunctorValue
    functor = 'T'
    value_type = 'min'
  []
  [T_max]
    type = ADElementExtremeFunctorValue
    functor = 'T'
    value_type = 'max'
  []
[]

[Outputs]
  csv = true
[]
