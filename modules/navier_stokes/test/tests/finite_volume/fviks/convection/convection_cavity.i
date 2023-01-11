mu = 1
rho = 1
k = .01
cp = 1
velocity_interp_method = 'rc'
advected_interp_method = 'average'

[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 0.5'
    dy = '1'
    ix = '8 5'
    iy = '8'
    subdomain_id = '0 1'
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'cmg'
    primary_block = 0
    paired_block = 1
    new_boundary = 'interface'
  []
  [secondary_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'interface'
    primary_block = 1
    paired_block = 0
    new_boundary = 'secondary_interface'
  []
[]

[GlobalParams]
  # retain behavior at time of test creation
  two_term_boundary_expansion = false
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = u
    v = v
    block = 0
    pressure = pressure
  []
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
    block = 0
  []
  [v]
    type = INSFVVelocityVariable
    block = 0
  []
  [pressure]
    type = INSFVPressureVariable
    block = 0
  []
  [T]
    type = INSFVEnergyVariable
    block = 0
  []
  [Ts]
    type = INSFVEnergyVariable
    block = 1
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]

[ICs]
  [T]
    type = ConstantIC
    variable = T
    value = 1
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    block = 0
  []
  [mean_zero_pressure]
    type = FVIntegralValueConstraint
    variable = pressure
    lambda = lambda
    block = 0
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    rho = ${rho}
    block = 0
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = u
    mu = ${mu}
    block = 0
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    pressure = pressure
    block = 0
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = v
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    rho = ${rho}
    block = 0
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = v
    mu = ${mu}
    block = 0
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    pressure = pressure
    block = 0
  []

  [temp_conduction]
    type = FVDiffusion
    coeff = 'k'
    variable = T
    block = 0
  []
  [temp_advection]
    type = INSFVEnergyAdvection
    variable = T
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    block = 0
  []

  [solid_temp_conduction]
    type = FVDiffusion
    coeff = 'k'
    variable = Ts
    block = 1
  []
[]

[FVInterfaceKernels]
  [convection]
    type = FVConvectionCorrelationInterface
    variable1 = T
    variable2 = Ts
    boundary = 'interface'
    h = 5
    T_solid = Ts
    T_fluid = T
    subdomain1 = 0
    subdomain2 = 1
    bulk_distance = 0.3
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
    boundary = 'left interface bottom'
    function = 0
  []

  [no_slip_y]
    type = INSFVNoSlipWallBC
    variable = v
    boundary = 'left interface top bottom'
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
    variable = Ts
    boundary = 'right'
    value = 0
  []
[]

[Materials]
  [functor_constants]
    type = ADGenericFunctorMaterial
    prop_names = 'cp k'
    prop_values = '${cp} ${k}'
  []
  [ins_fv]
    type = INSFVEnthalpyMaterial
    temperature = 'T'
    rho = ${rho}
    block = 0
  []
[]

[Functions]
  [lid_function]
    type = ParsedFunction
    expression = '4*x*(1-x)'
  []
[]


[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -ksp_gmres_restart'
  petsc_options_value = 'asm      lu           NONZERO                   200'
  line_search = 'none'
  nl_rel_tol = 1e-12
  nl_max_its = 6
  l_max_its = 200
[]

[Outputs]
  exodus = true
[]
