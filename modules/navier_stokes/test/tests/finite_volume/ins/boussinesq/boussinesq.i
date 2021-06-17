mu = 1
rho = 1
k = 1
cp = 1
alpha = 1
vel = 'velocity'
velocity_interp_method = 'rc'
advected_interp_method = 'upwind'
rayleigh=1e3
hot_temp=${rayleigh}
temp_ref=${fparse hot_temp / 2.}

[GlobalParams]
  two_term_boundary_expansion = true
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 32
    ny = 32
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
    scaling = 1e-4
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]

[AuxVariables]
  [U]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
  [vel_x]
    order = FIRST
    family = MONOMIAL
  []
  [vel_y]
    order = FIRST
    family = MONOMIAL
  []
  [viz_T]
    order = FIRST
    family = MONOMIAL
  []
[]

[AuxKernels]
  [mag]
    type = VectorMagnitudeAux
    variable = U
    x = u
    y = v
    execute_on = 'initial timestep_end'
  []
  [vel_x]
    type = ParsedAux
    variable = vel_x
    function = 'u'
    execute_on = 'initial timestep_end'
    args = 'u'
  []
  [vel_y]
    type = ParsedAux
    variable = vel_y
    function = 'v'
    execute_on = 'initial timestep_end'
    args = 'v'
  []
  [viz_T]
    type = ParsedAux
    variable = viz_T
    function = 'T'
    execute_on = 'initial timestep_end'
    args = 'T'
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    vel = ${vel}
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    u = u
    v = v
    pressure = pressure
    mu = ${mu}
    rho = ${rho}
  []
  [mean_zero_pressure]
    type = FVScalarLagrangeMultiplier
    variable = pressure
    lambda = lambda
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    advected_quantity = 'rhou'
    vel = ${vel}
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
  []
  [u_viscosity]
    type = FVDiffusion
    variable = u
    coeff = ${mu}
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    p = pressure
  []
  [u_buoyancy]
    type = INSFVMomentumBoussinesq
    variable = u
    temperature = T
    gravity = '0 -1 0'
    rho = ${rho}
    ref_temperature = ${temp_ref}
    momentum_component = 'x'
  []
  [u_gravity]
    type = INSFVMomentumGravity
    variable = u
    gravity = '0 -1 0'
    rho = ${rho}
    momentum_component = 'x'
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = v
    advected_quantity = 'rhov'
    vel = ${vel}
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
  []
  [v_viscosity]
    type = FVDiffusion
    variable = v
    coeff = ${mu}
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    p = pressure
  []
  [v_buoyancy]
    type = INSFVMomentumBoussinesq
    variable = v
    temperature = T
    gravity = '0 -1 0'
    rho = ${rho}
    ref_temperature = ${temp_ref}
    momentum_component = 'y'
  []
  [v_gravity]
    type = INSFVMomentumGravity
    variable = v
    gravity = '0 -1 0'
    rho = ${rho}
    momentum_component = 'y'
  []

  [temp_conduction]
    type = FVDiffusion
    coeff = 'k'
    variable = T
  []
  [temp_advection]
    type = INSFVEnergyAdvection
    variable = T
    vel = ${vel}
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
  []
[]

[FVBCs]
  [top_x]
    type = INSFVNoSlipWallBC
    variable = u
    boundary = 'top'
    function = 'lid_function'
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
    boundary = left
    value = ${hot_temp}
  []

  [T_cold]
    type = FVDirichletBC
    variable = T
    boundary = right
    value = 0
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'k cp alpha'
    prop_values = '${k} ${cp} ${alpha}'
  []
  [ins_fv]
    type = INSFVMaterial
    u = 'u'
    v = 'v'
    pressure = 'pressure'
    temperature = 'T'
    rho = ${rho}
  []
[]

[Functions]
  [lid_function]
    type = ParsedFunction
    value = '4*x*(1-x)'
  []
[]


[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      300                lu           NONZERO'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
