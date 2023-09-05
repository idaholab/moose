D = 0.1
A = '${fparse (1./4.)*pi*D^2}'
P_hf = '${fparse pi*D}'
D_h = '${fparse 4*A/P_hf}'
mdot = 0.04
file_base = 'db_churchill'

[GlobalParams]
  gravity_vector = '0 0 0'

  initial_vel = 0.003
  initial_p = 1e5
  initial_T = 300

  D_h = ${D_h}
  A = ${A}
  P_hf = ${P_hf}

  m_dot = ${mdot}

  closures = thm
  execute_on = 'initial timestep_begin'
[]

[FluidProperties]
  [water]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
    k = 0.56361
    mu = 8.84e-05
  []
[]

[Closures]
  [thm]
    type = Closures1PhaseTHM
    wall_htc_closure = dittus_boelter
    wall_ff_closure = churchill
  []
[]

[Components]
  [pipe]
    type = FlowChannel1Phase
    fp = water
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 10
  []

  #--------------Pipe BCs-------------#
  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'pipe:in'
    T = 300
  []
  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 1e5
  []
  [ht]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = 'pipe'
    T_wall = 500
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
  num_steps = 1
  dt = 1e-5
[]

[Postprocessors]

  [Hw]
    type = ADElementAverageMaterialProperty
    mat_prop = Hw
  []
  [f]
    type = ADElementAverageMaterialProperty
    mat_prop = f_D
    block = pipe
  []
[]

[Outputs]
  csv = true
  file_base = ${file_base}
[]
