[GlobalParams]
  scaling_factor_1phase = '1 1 1e-3'
[]

[HeatStructureMaterials]
  [mat]
    type = SolidMaterialProperties
    k = 30
    cp = 100
    rho = 1000
  []
[]

[Modules/FluidProperties]
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

[Functions]
  [T_init]
    type = ParsedFunction
    value = '1000*y+300+30*z'
  []
[]

[Components]
  [fch1]
    type = FlowChannel1Phase
    position = '0 0 0'
    # orientation = '0 0 1'
    fp = fp
    n_elems = 6
    length = 1
    initial_T = 300
    initial_p = 1.01e5
    initial_vel = 1
    closures = simple_closures
    A   = 0.00314159
    D_h  = 0.2
    f = 0.01
  []
  [in1]
    type = InletVelocityTemperature1Phase
    input = 'fch1:in'
    vel = 1
    T = 300
  []
  [out1]
    type = Outlet1Phase
    input = 'fch1:out'
    p = 1.01e5
  []

  [fch2]
    type = FlowChannel1Phase
    position = '0.2 0.2 0'
    # orientation = '0 0 1'
    fp = fp
    n_elems = 6
    length = 1
    initial_T = 300
    initial_p = 1.01e5
    initial_vel = 1
    closures = simple_closures
    A   = 0.00314159
    D_h  = 0.2
    f = 0.01
  []
  [in2]
    type = InletVelocityTemperature1Phase
    input = 'fch2:in'
    vel = 1
    T = 300
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
    initial_T = T_init
    materials = 'mat'
  []
  [ht]
    type = HeatTransferFromHeatStructure3D1Phase
    flow_channels = 'fch1 fch2'
    hs = blk
    boundary = blk:rmin
    Hw = 10000
    P_hf = 0.156434465
  []
[]

[Postprocessors]
  [energy_hs]
    type = HeatStructureEnergy3D
    block = blk:0
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [energy_fch]
    type = ElementIntegralVariablePostprocessor
    block = 'fch1 fch2'
    variable = rhoEA
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [total_energy]
    type = SumPostprocessor
    values = 'energy_fch energy_hs'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [energy_change]
    type = ChangeOverTimePostprocessor
    change_with_respect_to_initial = true
    postprocessor = total_energy
   compute_relative_change = false
    execute_on = 'INITIAL TIMESTEP_END'
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
  dt = 1
  solve_type = PJFNK
  line_search = basic
  num_steps = 1000
  steady_state_detection = true
  steady_state_tolerance = 1e-08
  nl_abs_tol = 1e-8
[]

[Outputs]
  exodus = true
[]
