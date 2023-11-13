# ========================================================================
# Author: Maricio E. Tano @ INL
# Date  : 5/17/2023
# ========================================================================
# The purpose of this MOOSE scripts is to solve a 2-D axisymmetric
#     roblem with the following details:
#     ------------------------------------------------------------------
#     Physics: natural convection through a fluid  and heat conduction
#              in a solid and there is convective heat transfer from the
#              solid to the liquid.
#     ------------------------------------------------------------------
#     Materials: the fluid is water and the solid is not specified.
#     ------------------------------------------------------------------

#     BCS: Inlet and outlet pressure with value of 0
#          noslip conditions on the walls.
#          Heat flux on the left wall with value of 40000 W/m^2
# ========================================================================

# ========================================================================
#           Dimensions & Physical properties
# ========================================================================

Domain_length = 121.92e-2 # m
Solid_width = 0.7112e-3 # m
Liquid_width = 0.56261e-2 # m

mu = 0.00053157
rho = 987.27
k = 0.64247
k_solid = 15.0
cp = 4181.8

alpha_b = 210e-6
Tb = 0.5
# ========================================================================
#             The main body of the script
# ========================================================================
[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    #dx   = '0.7032625e-4  0.7112e-5'
    dx = '${Liquid_width} ${Solid_width}'
    ix = '80 20'
    dy = '${Domain_length}'
    iy = '200'
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
    u = vel_x
    v = vel_y
    block = 0
    pressure = pressure
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    block = 0
    initial_condition = 1e-6
  []
  [vel_y]
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
    initial_condition = 0.5
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
    variable = vel_x
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
  [u_buoyancy]
    type = INSFVMomentumBoussinesq
    variable = vel_x
    T_fluid = T
    gravity = '0 -9.81 0'
    rho = ${rho}
    ref_temperature = 0.5
    momentum_component = 'x'
  []
  #[u_gravity]
  #  type = INSFVMomentumGravity
  #  variable = vel_x
  ##  gravity = '0 -9.81 0'
  #  rho = ${rho}
  #  momentum_component = 'x'
  #[]
  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
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
  [v_buoyancy]
    type = INSFVMomentumBoussinesq
    variable = vel_y
    T_fluid = T
    gravity = '0 -9.81 0'
    rho = ${rho}
    ref_temperature = 0.5
    momentum_component = 'y'
  []
  #[v_gravity]
  #  type = INSFVMomentumGravity
  #  variable = vel_y
  #  gravity = '0 -9.81 0'
  #  rho = ${rho}
  #  momentum_component = 'y'
  #[]

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
    coeff = 'k_solid'
    variable = Ts
  []
[]

[FVInterfaceKernels]
  [convection]
    type = FVConvectionCorrelationInterface
    variable1 = T
    variable2 = Ts
    boundary = 'interface'
    h = 'htc'
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
    variable = vel_x
    boundary = 'interface left'
    function = 0
  []
  [walls_v]
    type = INSFVNoSlipWallBC
    variable = vel_y
    boundary = 'interface left'
    function = 0
  []

  [inlet_u]
    type = INSFVInletVelocityBC
    variable = vel_x
    boundary = 'bottom_to_0'
    function = 0
  []
  [inlet_v]
    type = INSFVInletVelocityBC
    variable = vel_y
    boundary = 'bottom_to_0'
    function = 0
  []

  [inlet_T]
    type = FVDirichletBC
    variable = T
    boundary = 'bottom_to_0'
    value = 0.5
  []

  [outlet]
    type = INSFVOutletPressureBC
    variable = pressure
    boundary = 'top_to_0'
    function = 0.0
  []

  [heater]
    type = FVNeumannBC
    variable = 'Ts'
    boundary = 'right'
    value = 62259.716407
  []
  [Insulator]
    type = FVNeumannBC
    variable = 'T'
    boundary = 'left'
    value = 0.0
  []
[]

[AuxVariables]
  [Ra]
    type = MooseVariableFVReal
    initial_condition = 1.0
  []
  [Pr]
    type = MooseVariableFVReal
    initial_condition = 1.0
  []
  [htc]
    type = MooseVariableFVReal
    initial_condition = 1.0
  []
[]

[AuxKernels]
  [compute_Ra]
    type = ParsedAux
    variable = Ra
    coupled_variables = 'T'
    constant_names = 'g beta Tb width nu alpha'
    constant_expressions = '9.81 ${alpha_b} ${Tb} ${Liquid_width} ${fparse mu/rho} ${fparse k/(rho*cp)}'
    expression = 'g * beta * (T - Tb) * pow(width, 3) / (nu*alpha)'
    block = 0
  []
  [compute_Pr]
    type = ParsedAux
    variable = Pr
    constant_names = 'mu cp k'
    constant_expressions = '${mu} ${cp} ${k}'
    expression = 'mu * cp / k'
    block = 0
  []
  [compute_htc]
    type = ParsedAux
    variable = htc
    coupled_variables = 'Pr Ra'
    expression = 'Pr * Ra'
    block = 0
  []
[]

[Materials]
  [functor_constants]
    type = ADGenericFunctorMaterial
    prop_names = 'cp k k_solid'
    prop_values = '${cp} ${k} ${k_solid}'
  []
  [ins_fv]
    type = INSFVEnthalpyMaterial
    temperature = 'T'
    rho = ${rho}
    block = 0
  []
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'alpha_b'
    prop_values = '${alpha_b}'
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
    vel_x = vel_x
    vel_y = vel_y
    advected_quantity = ${rho}
  []
[]

[Outputs]
  exodus = true
[]
