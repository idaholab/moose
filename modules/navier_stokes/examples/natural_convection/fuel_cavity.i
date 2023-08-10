# ========================================================================
# Author: Fadel M. Nasr @ INL
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

Domain_length = 121.92e-3 #2 # m
Solid_width = 0.7112e-3 # m
Liquid_width = 0.56261e-2 # m

mu = 0.00053157
rho = 987.27
k = 0.64247
k_solid = 15.0
cp = 4181.8

alpha_b = 210e-6
#Tb = 273.0
T_init = 300.0

input_heat_flux = 40000.0

# ========================================================================
#             The main body of the script
# ========================================================================
[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    #dx   = '0.7032625e-4  0.7112e-5'
    dx = '${Liquid_width} ${Solid_width}'
    ix = '20 5'
    dy = '${fparse Domain_length/5} ${fparse 4*Domain_length/5}'
    iy = '80 30'
    subdomain_id = '0 1
                    0 1'
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
  advected_interp_method = 'upwind'
  velocity_interp_method = 'average'
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
    initial_condition = 1e-6
  []
  [T]
    type = INSFVEnergyVariable
    block = 0
    initial_condition = ${T_init}
    scaling = 1e-3
  []
  [Ts]
    type = INSFVEnergyVariable
    block = 1
    scaling = 1e-3
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]

[FVKernels]

  # inactive = 'u_time v_time energy_time'

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

  [u_time]
    type = INSFVMomentumTimeDerivative
    variable = vel_x
    rho = ${rho}
    momentum_component = 'x'
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
    ref_temperature = ${T_init}
    momentum_component = 'x'
    #alpha_name = ${alpha_b}
  []

  [v_time]
    type = INSFVMomentumTimeDerivative
    variable = vel_y
    rho = ${rho}
    momentum_component = 'x'
  []
  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    rho = ${rho}
    momentum_component = 'y'
    #alpha_name = ${alpha_b}
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
    ref_temperature = ${T_init}
    momentum_component = 'y'
  []

  [energy_time]
    type = INSFVEnergyTimeDerivative
    variable = T
    cp = ${cp}
    rho = ${rho}
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
    h = 10.0 #htc
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
    boundary = 'interface left bottom_to_0'
    function = 0
  []
  [walls_v]
    type = INSFVNoSlipWallBC
    variable = vel_y
    boundary = 'interface left bottom_to_0'
    function = 0
  []
  # [inlet_u]
  #   type = INSFVInletVelocityBC
  #   variable = vel_x
  #   boundary = 'bottom_to_0'
  #   function = 0
  # []
  # [inlet_v]
  #   type = INSFVInletVelocityBC
  #   variable = vel_y
  #   boundary = 'bottom_to_0'
  #   function = 0
  # []
  # [inlet_T_0]
  #   type = FVDirichletBC
  #   variable = T
  #   boundary = 'bottom_to_0'
  #   value = ${T_init}
  # []
  # [inlet_T_1]
  #   type = FVDirichletBC
  #   variable = Ts
  #   boundary = 'bottom_to_1'
  #   value = ${T_init}
  # []
  [outlet]
    type = INSFVOutletPressureBC
    variable = pressure
    boundary = 'top_to_0'
    function = 0.0
  []
  [outlet_T]
    type = NSFVOutflowTemperatureBC
    variable = T
    boundary = 'top_to_0'
    advected_quantity = '${fparse rho*cp}'
    u = vel_x
    v = vel_y
    backflow_T = ${T_init}
  []

  [heater]
    type = FVNeumannBC
    variable = 'Ts'
    boundary = 'right'
    value = '${fparse input_heat_flux/k_solid}'
  []
  [Insulator]
    type = FVNeumannBC
    variable = 'T'
    boundary = 'bottom_to_0 left'
    value = 0.0
  []
  [Insulator_solid]
    type = FVNeumannBC
    variable = 'Ts'
    boundary = 'bottom_to_1 top_to_1'
    value = 0.0
  []

[]

[AuxVariables]
  [Ra]
    type = INSFVScalarFieldVariable
    initial_condition = 1000.0
  []
  [htc]
    type = INSFVScalarFieldVariable
    initial_condition = 500.0
  []
[]

[AuxKernels]
  [compute_Ra]
    type = ParsedAux
    variable = Ra
    coupled_variables = 'T'
    constant_names = 'g beta T_init width nu alpha'
    constant_expressions = '9.81 ${alpha_b} ${T_init} ${Liquid_width} ${fparse mu/rho} ${fparse k/(rho*cp)}'
    expression = 'g * beta * (T - T_init) * pow(width, 3) / (nu*alpha)'
    block = 0
  []
  [htc]
    type = ParsedAux
    variable = htc
    coupled_variables = 'Ra'
    constant_names = 'Pr'
    constant_expressions = '${fparse cp*mu/k}'
    #expression = '0.1 * Ra * Pr'
    expression = '${k}* (0.68 + 0.67 * pow(Ra, 0.25)/pow(1 + pow(0.437/Pr, 9/16) ,4/9) )/ ${Liquid_width} '
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

[Debug]
  show_var_residual_norms = true
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_shift_type -ksp_gmres_restart -snes_linesearch_damping'
  petsc_options_value = 'lu        NONZERO                   200            1.0'
  line_search = 'none'
  nl_abs_tol = 1e-14

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.1
    optimal_iterations = 20
    iteration_window = 2
    growth_factor = 2
    cutback_factor = 0.5
  []
  end_time = 1e+30
  steady_state_tolerance = 1e-04
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
