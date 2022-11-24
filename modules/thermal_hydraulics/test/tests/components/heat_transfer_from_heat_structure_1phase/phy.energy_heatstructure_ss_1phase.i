# This test tests conservation of energy at steady state for 1-phase flow when a
# heat structure is used. Conservation is checked by comparing the integral of
# the heat flux against the difference of the boundary fluxes.

[GlobalParams]
  initial_p = 7.0e6
  initial_vel = 0
  initial_T = 513

  gravity_vector = '0.0 0.0 0.0'

  scaling_factor_1phase = '1 1 1e-4'

  closures = simple_closures
[]

[FluidProperties]
  [eos]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    q = -1167e3
    q_prime = 0
    p_inf = 1.e9
    cv = 1816
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[HeatStructureMaterials]
  [fuel-mat]
    type = SolidMaterialProperties
    k = 3.7
    cp = 3.e2
    rho = 10.42e3
  []
  [gap-mat]
    type = SolidMaterialProperties
    k = 0.7
    cp = 5e3
    rho = 1.0
  []
  [clad-mat]
    type = SolidMaterialProperties
    k = 16
    cp = 356.
    rho = 6.551400E+03
  []
[]

[Components]
  [reactor]
    type = TotalPower
    power = 1e3
  []

  [core:pipe]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '0 0 1'
    length = 3.66
    n_elems = 10

    A = 1.907720E-04
    D_h = 1.698566E-02
    f = 0.0

    fp = eos
  []

  [core:solid]
    type = HeatStructureCylindrical
    position = '0 -0.0071501 0'
    orientation = '0 0 1'
    length = 3.66
    n_elems = 10

    names =  'FUEL GAP CLAD'
    widths = '6.057900E-03  1.524000E-04  9.398000E-04'
    n_part_elems = '5 1 2'
    materials = 'fuel-mat gap-mat clad-mat'

    initial_T = 513
  []

  [core:hgen]
    type = HeatSourceFromTotalPower
    hs = core:solid
    regions = 'FUEL'
    power = reactor
    power_fraction = 1
  []

  [core:hx]
    type = HeatTransferFromHeatStructure1Phase
    flow_channel = core:pipe
    hs   = core:solid
    hs_side = outer
    Hw = 1.0e4
    P_hf = 4.4925e-2
  []

  [inlet]
    type = InletDensityVelocity1Phase
    input = 'core:pipe:in'
    rho = 817.382210128610836
    vel = 2.4
  []

  [outlet]
    type = Outlet1Phase
    input = 'core:pipe:out'
    p = 7e6
  []
[]

[Postprocessors]
  [E_in]
    type = ADFlowBoundaryFlux1Phase
    boundary = inlet
    equation = energy
    execute_on = 'initial timestep_end'
  []

  [E_out]
    type = ADFlowBoundaryFlux1Phase
    boundary = outlet
    equation = energy
    execute_on = 'initial timestep_end'
  []

  [hf_pipe]
    type = ADHeatRateConvection1Phase
    block = core:pipe
    T_wall = T_wall
    T = T
    Hw = Hw
    P_hf = P_hf
    execute_on = 'initial timestep_end'
  []

  [E_diff]
    type = DifferencePostprocessor
    value1 = E_in
    value2 = E_out
    execute_on = 'initial timestep_end'
  []

  [E_conservation]
    type = SumPostprocessor
    values = 'E_diff hf_pipe'
  []
[]

[Preconditioning]
  [SMP_PJFNK]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  abort_on_solve_fail = true
  dt = 5

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  nl_max_its = 50

  l_tol = 1e-3
  l_max_its = 60

  start_time = 0
  end_time = 260
[]

[Outputs]
  [out]
    type = CSV
    execute_on = final
    show = 'E_conservation'
  []
  [console]
    type = Console
    show = 'E_conservation'
  []
[]
