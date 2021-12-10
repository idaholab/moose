[GlobalParams]
  scaling_factor_1phase = '1 1e-1 1e-2'
  scaling_factor_temperature = 1e-2
  gravity_vector = '0 0 0'
[]

[HeatStructureMaterials]
  [mat]
    type = SolidMaterialProperties
    k = 30
    cp = 100
    rho = 1000
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
  [fch]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '0 0 1'
    fp = fp
    n_elems = 10
    length = 1
    initial_T = 300
    initial_p = 1.01e5
    initial_vel = 1
    closures = simple_closures
    A  = 0.00314159
    f = 0.0
  []
  [in]
    type = InletVelocityTemperature1Phase
    input = 'fch:in'
    vel = 1
    T = 300
  []
  [out]
    type = Outlet1Phase
    input = 'fch:out'
    p = 1.01e5
  []
  [blk]
    type = HeatStructureFromFile3D
    file = mesh.e
    position = '0 0 0'
    initial_T = 400
    materials = 'mat'
  []
  [rmax]
    type = HSBoundaryAmbientConvection
    boundary = blk:rmax
    hs = blk
    htc_ambient = 1e5
    T_ambient = 400
  []
  [ht]
    type = HeatTransferFromHeatStructure3D1Phase
    flow_channels = 'fch'
    hs = blk
    boundary = blk:rmin
    Hw = 10000
    P_hf = 0.1564344650402309
  []
[]

[Postprocessors]
  [E_in]
    type = ADFlowBoundaryFlux1Phase
    boundary = in
    equation = energy
    execute_on = 'initial timestep_end'
  []
  [E_out]
    type = ADFlowBoundaryFlux1Phase
    boundary = out
    equation = energy
    execute_on = 'initial timestep_end'
  []
  [hf_pipe]
    type = ADHeatRateConvection1Phase
    block = fch
    T_wall = T_wall
    T = T
    Hw = Hw
    P_hf = 0.1564344650402309
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
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  dt = 5
  solve_type = PJFNK
  line_search = basic
  end_time = 150
  nl_abs_tol = 1e-8
[]

[Outputs]
  file_base = 'phy.conservation_ss'
  [csv]
    type = CSV
    show = 'E_conservation'
    execute_on = 'FINAL'
  []
[]
