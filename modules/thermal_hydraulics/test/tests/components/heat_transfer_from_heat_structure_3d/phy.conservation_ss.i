# Testing energy conservation at steady state

P_hf = ${fparse 0.6 * sin (pi/24)}

[GlobalParams]
  scaling_factor_1phase = '1 1 1e-3'
  gravity_vector = '0 0 0'
[]

[Materials]
  [mat]
    type = ADGenericConstantMaterial
    block = 'blk:0'
    prop_names = 'density specific_heat thermal_conductivity'
    prop_values = '1000 10 30'
  []
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]


[Components]
  [in1]
    type = InletVelocityTemperature1Phase
    input = 'fch1:in'
    vel = 1
    T = 300
  []
  [fch1]
    type = FlowChannel1Phase
    position = '0.15 0 0'
    orientation = '0 0 1'
    fp = fp
    n_elems = 10
    length = 1
    initial_T = 300
    initial_p = 1.01e5
    initial_vel = 1
    closures = simple_closures
    A = 0.00314159
    f = 0.0
  []
  [out1]
    type = Outlet1Phase
    input = 'fch1:out'
    p = 1.01e5
  []

  [in2]
    type = InletVelocityTemperature1Phase
    input = 'fch2:in'
    vel = 1
    T = 350
  []
  [fch2]
    type = FlowChannel1Phase
    position = '0 0.15 0'
    orientation = '0 0 1'
    fp = fp
    n_elems = 10
    length = 1
    initial_T = 350
    initial_p = 1.01e5
    initial_vel = 1
    closures = simple_closures
    A = 0.00314159
    f = 0
  []
  [out2]
    type = Outlet1Phase
    input = 'fch2:out'
    p = 1.01e5
  []

  [blk]
    type = HeatStructureFromFile3D
    file = mesh.e
    position = '0 0 0'
    initial_T = 325
  []
  [ht]
    type = HeatTransferFromHeatStructure3D1Phase
    flow_channels = 'fch1 fch2'
    hs = blk
    boundary = blk:rmin
    Hw = 10000
    P_hf = ${P_hf}
  []
[]

[Postprocessors]
  [E_in1]
    type = ADFlowBoundaryFlux1Phase
    boundary = in1
    equation = energy
    execute_on = 'initial timestep_end'
  []
  [E_out1]
    type = ADFlowBoundaryFlux1Phase
    boundary = out1
    equation = energy
    execute_on = 'initial timestep_end'
  []
  [hf_pipe1]
    type = ADHeatRateConvection1Phase
    block = fch1
    T_wall = T_wall
    T = T
    Hw = Hw
    P_hf = ${P_hf}
    execute_on = 'initial timestep_end'
  []
  [E_diff1]
    type = DifferencePostprocessor
    value1 = E_in1
    value2 = E_out1
    execute_on = 'initial timestep_end'
  []
  [E_conservation1]
    type = SumPostprocessor
    values = 'E_diff1 hf_pipe1'
  []
  [E_in2]
    type = ADFlowBoundaryFlux1Phase
    boundary = in2
    equation = energy
    execute_on = 'initial timestep_end'
  []
  [E_out2]
    type = ADFlowBoundaryFlux1Phase
    boundary = out2
    equation = energy
    execute_on = 'initial timestep_end'
  []
  [hf_pipe2]
    type = ADHeatRateConvection1Phase
    block = fch2
    T_wall = T_wall
    T = T
    Hw = Hw
    P_hf = ${P_hf}
    execute_on = 'initial timestep_end'
  []
  [E_diff2]
    type = DifferencePostprocessor
    value1 = E_in2
    value2 = E_out2
    execute_on = 'initial timestep_end'
  []
  [E_conservation2]
    type = SumPostprocessor
    values = 'E_diff2 hf_pipe2'
  []
  [E_conservation_hs]
    type = SumPostprocessor
    values = 'hf_pipe1 hf_pipe2'
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  dt = 5
  end_time = 100

  solve_type = NEWTON
  line_search = basic
  abort_on_solve_fail = true
  nl_abs_tol = 1e-8
[]

[Outputs]
  file_base = 'phy.conservation_ss'
  [csv]
    type = CSV
    show = 'E_conservation1 E_conservation2 E_conservation_hs'
    execute_on = 'FINAL'
  []
[]
