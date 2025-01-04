# Tests that a flow channel can run with Steady executioner and be set up using Physics
#
# Note that this solve may fail to converge based on initial guess. For example,
# having a guess with velocity set to zero will fail to converge.

L = 30

bulk_u = 0.01
p_ref = 101325
T_inlet = 860
q_source = 50000 # W/m3

A_cp = 976.78
B_cp = 1.0634
rho = 2000

[Functions]
  [cp_fun]
    type = ParsedFunction
    expression = 'A_cp + B_cp*x'
    symbol_names = 'A_cp B_cp'
    symbol_values = '${A_cp} ${B_cp}'
  []
[]

[FluidProperties]
  [fp]
    type = TemperaturePressureFunctionFluidProperties
    rho = ${rho}
    cp = cp_fun
    k = 0.7
    mu = 4.5e-3
  []
[]

[FunctorMaterials]
  [functor_fluid_props]
    type = GeneralFunctorFluidProps
    fp = fp
    T_fluid = T_fluid
    pressure = 'pressure'
    characteristic_length = 1
    porosity = 1
    speed = 1 # Re unused
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
    add_functor_materials = true
    add_regular_materials = false
    outputs = 'all'
  []
[]


[Physics]
  [ThermalHydraulics]
    [WeaklyCompressibleFlow]
      [flow]
      []
    []
    [WeaklyCompressibleFluidHeatTransfer]
      [energy]
      []
    []
  []
[]

[Components]
  # [inlet]
  #   type = InletMassFlowRateTemperature
  #   input = 'pipe:in'
  #   m_dot = ${fparse rho * bulk_u * 1}
  #   T = ${T_inlet}
  # []
  [inlet]
    type = InletVelocityTemperature
    input = 'pipe:in'
    vel = ${bulk_u}
    T = ${T_inlet}
  []

  [pipe]
    type = FlowChannel
    position = '0 0 0'
    orientation = '1 0 0'
    gravity_vector = '0 0 0'
    length = ${L}
    n_elems = 100
    A = 1.0

    initial_T = ${T_inlet}
    initial_p = ${p_ref}
    initial_vel = ${bulk_u}

    physics = 'flow energy'
    f = 0
    closures = simple_closures
    fp = fp
  []
 [ht]
   type = HeatTransferFromHeatFlux
   flow_channel = pipe
   # This is supposed to a wall heat flux in W/m2
   # Divide by the heated perimeter (2pi r = 2pi sqrt(A/pi))
   q_wall = ${fparse q_source / (3.141592653589793)^0.5 / 2}
   # Used to compute T_wall, unused here
   Hw = '1e8'
 []

  [outlet]
    type = Outlet
    input = 'pipe:out'
    p = ${p_ref}
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady

  solve_type = NEWTON
  nl_rel_tol = 1e-7
  nl_abs_tol = 1e-7
  nl_max_its = 15

  l_tol = 1e-4
  l_max_its = 10

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  automatic_scaling = true
  off_diagonals_in_auto_scaling = true
[]

[Outputs]
  exodus = true
[]


[Postprocessors]
  [pressure_left]
    type = SideAverageValue
    variable = pressure
    boundary = pipe:in
    execute_on = 'initial timestep_end'
  []
  [pressure_right]
    type = SideAverageValue
    variable = pressure
    boundary = pipe:out
    execute_on = 'initial timestep_end'
  []
  [temperature_left]
    type = SideAverageValue
    variable = T_fluid
    boundary = pipe:in
    execute_on = 'initial timestep_end'
  []
  [temperature_right]
    type = SideAverageValue
    variable = T_fluid
    boundary = pipe:out
    execute_on = 'initial timestep_end'
  []
  [mass_right]
    type = VolumetricFlowRate
    boundary = pipe:out
    vel_x = vel_x
    advected_quantity = 'rho'
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
    execute_on = 'initial timestep_end'
  []
  [vel_left]
    type = SideAverageValue
    variable = 'vel_x'
    boundary = 'pipe:in'
  []
  [vel_right]
    type = SideAverageValue
    variable = 'vel_x'
    boundary = 'pipe:out'
  []

  [T_analytical]
    type = Receiver
    default = ${fparse (-A_cp+sqrt(A_cp^2-2*B_cp*(-q_source/rho/bulk_u*L-A_cp*T_inlet-B_cp/2*T_inlet*T_inlet)))/B_cp}
  []
  [error_T]
    type = ParsedPostprocessor
    expression = 'temperature_right - T_analytical'
    pp_names = 'temperature_right T_analytical'
  []
[]
