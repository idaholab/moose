mu = 1
rho = 1
k = .01
cp = 1

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
  [fluid_side]
    type = BreakBoundaryOnSubdomainGenerator
    input = 'interface'
    boundaries = 'top bottom'
  []
[]

[GlobalParams]
  # retain behavior at time of test creation
  two_term_boundary_expansion = false
  rhie_chow_user_object = 'rc'
  advected_interp_method = 'average'
  velocity_interp_method = 'rc'
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
    initial_condition = 1e-6
  []
  [v]
    type = INSFVVelocityVariable
    block = 0
    initial_condition = 1e-6
  []
  [pressure]
    type = INSFVPressureVariable
    block = 0
  []
  [T]
    type = INSFVEnergyVariable
    block = 0
    initial_condition = 1
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
    mu = ${mu}
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
    mu = ${mu}
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
    coeff = 'k'
    variable = T
  []
  [temp_advection]
    type = INSFVEnergyAdvection
    variable = T
  []

  [solid_temp_conduction]
    type = FVDiffusion
    coeff = 'k'
    variable = Ts
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
    wall_cell_is_bulk = true
  []
[]

[FVBCs]
  [walls_u]
    type = INSFVNoSlipWallBC
    variable = u
    boundary = 'interface left'
    function = 0
  []
  [walls_v]
    type = INSFVNoSlipWallBC
    variable = v
    boundary = 'interface left'
    function = 0
  []

  [inlet_u]
    type = INSFVInletVelocityBC
    variable = u
    boundary = 'bottom_to_0'
    function = 0
  []
  [inlet_v]
    type = INSFVInletVelocityBC
    variable = v
    boundary = 'bottom_to_0'
    function = 1
  []

  [inlet_T]
    type = FVDirichletBC
    variable = T
    boundary = 'bottom_to_0'
    value = 0.5
  []

  [outlet]
    type = INSFVMassAdvectionOutflowBC
    variable = pressure
    boundary = 'top_to_0'
    u = u
    v = v
    rho = ${rho}
  []
  [outlet_u]
    type = INSFVMomentumAdvectionOutflowBC
    variable = u
    boundary = 'top_to_0'
    u = u
    v = v
    momentum_component = 'x'
    rho = ${rho}
  []
  [outlet_v]
    type = INSFVMomentumAdvectionOutflowBC
    variable = v
    boundary = 'top_to_0'
    u = u
    v = v
    momentum_component = 'y'
    rho = ${rho}
  []

  [heater]
    type = FVDirichletBC
    variable = 'Ts'
    boundary = 'right'
    value = 10
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

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -ksp_gmres_restart'
  petsc_options_value = 'asm      lu           NONZERO                   200'
  line_search = 'none'

  nl_abs_tol = 1e-14
[]

[Postprocessors]
  [max_T]
    type = ADElementExtremeFunctorValue
    functor = T
    block = 0
  []
  [max_Ts]
    type = ADElementExtremeFunctorValue
    functor = Ts
    block = 1
  []
  [mdot_out]
    type = VolumetricFlowRate
    boundary = 'top_to_0'
    vel_x = u
    vel_y = v
    advected_quantity = ${rho}
  []
[]

[Outputs]
  exodus = true
[]
