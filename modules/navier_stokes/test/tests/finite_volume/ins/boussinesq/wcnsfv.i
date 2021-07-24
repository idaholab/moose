mu = 1
rho = 'rho'
k = 1
cp = 1
alpha = 1
vel = 'velocity'
velocity_interp_method = 'rc'
advected_interp_method = 'average'
# rayleigh=1e3
cold_temp=300
hot_temp=310

[GlobalParams]
  two_term_boundary_expansion = true
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 10
    nx = 64
    ny = 64
  []
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
    initial_condition = 1e-15
  []
  [v]
    type = INSFVVelocityVariable
    initial_condition = 1e-15
  []
  [pressure]
    type = INSFVPressureVariable
    initial_condition = 1e5
  []
  [T]
    type = INSFVEnergyVariable
    scaling = 1e-4
    initial_condition = ${cold_temp}
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
  [rho]
    type = MooseVariableFVReal
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
  [rho]
    type = FunctorMatPropElementalAux
    mat_prop = 'rho'
    variable = 'rho'
    execute_on = 'initial timestep_end'
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
    phi0 = 1e5
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
    pressure = pressure
  []
  [u_gravity]
    type = NSFVMomentumGravity
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
    pressure = pressure
  []
  [v_gravity]
    type = NSFVMomentumGravity
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
  [no_slip_x]
    type = INSFVNoSlipWallBC
    variable = u
    boundary = 'left right top bottom'
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
    value = ${cold_temp}
  []
[]

[Modules]
  [FluidProperties]
    [fp]
      type = IdealGasFluidProperties
    []
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'k alpha'
    prop_values = '${k} ${alpha}'
  []
  [const_functor]
    type = ADGenericConstantFunctorMaterial
    prop_names = 'cp'
    prop_values = '${cp}'
  []
  [rho]
    type = RhoFromPTFunctorMaterial
    fp = fp
    temperature = T
    pressure = pressure
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
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
[]

[Outputs]
  exodus = true
[]
