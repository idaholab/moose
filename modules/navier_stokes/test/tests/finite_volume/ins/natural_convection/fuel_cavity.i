# ========================================================================
#     The purpose of this MOOSE scripts is to solve a 2-D axisymmetric
#     problem with the following details:
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
    ix = '10 3'
    dy = '${fparse 1./5.*Domain_length} ${fparse 4./5.*Domain_length}'
    iy = '30 10'
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
  rhie_chow_user_object = 'rc'
  advected_interp_method = 'upwind'
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
    initial_condition = ${T_init}
    scaling = 1e-5
  []
  [Ts]
    type = INSFVEnergyVariable
    block = 1
    initial_condition = ${T_init}
    scaling = 1e-3
  []
[]

[FVKernels]

  [mass]
    type = INSFVMassAdvection
    variable = pressure
    rho = ${rho}
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
    momentum_component = 'y'
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

  [temp_time]
    type = INSFVEnergyTimeDerivative
    variable = T
    rho = '${rho}'
    cp = '${cp}'
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

  [Ts_time]
    type = INSFVEnergyTimeDerivative
    variable = Ts
    rho = '${rho}'
    cp = '${cp}'
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
    h = htc
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
    u = vel_x
    v = vel_y
    rho = ${rho}
    cp = '${cp}'
    backflow_T = ${T_init}
  []
  [Insulator]
    type = FVNeumannBC
    variable = 'T'
    boundary = 'left'
    value = 0.0
  []

  [heater]
    type = FVNeumannBC
    variable = 'Ts'
    boundary = 'right'
    value = '${fparse input_heat_flux}'
  []
  [Insulator_solid]
    type = FVNeumannBC
    variable = 'Ts'
    boundary = 'top_to_1'
    value = 0.0
  []
  [inlet_T_1]
    type = FVDirichletBC
    variable = Ts
    boundary = 'bottom_to_1'
    value = ${T_init}
  []

[]

[AuxVariables]
  [Ra]
    type = INSFVScalarFieldVariable
    initial_condition = 1000.0
  []
  [htc]
    type = INSFVScalarFieldVariable
    initial_condition = 0.0
  []
[]

[AuxKernels]
  [compute_Ra]
    type = ParsedAux
    variable = Ra
    coupled_variables = 'T'
    constant_names = 'g beta T_init width nu alpha'
    constant_expressions = '9.81 ${alpha_b} ${T_init} ${Liquid_width} ${fparse mu/rho} ${fparse k/(rho*cp)}'
    expression = 'g * beta * (T - T_init) * pow(width, 3) / (nu*alpha) + 1.0'
    block = 0
  []
  [htc]
    type = ParsedAux
    variable = htc
    coupled_variables = 'Ra'
    constant_names = 'Pr'
    constant_expressions = '${fparse cp*mu/k}'
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

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -sub_pc_factor_shift_type -ksp_gmres_restart'
  petsc_options_value = ' lu       NONZERO                   200'
  line_search = 'none'

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.01
    optimal_iterations = 20
    iteration_window = 2
  []

  nl_max_its = 30
  nl_abs_tol = 1e-10

  steady_state_detection = true
  steady_state_tolerance = 1e-09
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
[]

[Outputs]
  exodus = false
  csv = true
[]
