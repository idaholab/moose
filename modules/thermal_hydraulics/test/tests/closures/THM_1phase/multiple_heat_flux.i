D = 0.1
A = ${fparse 0.25*pi*D^2}
P_hf = ${fparse pi*D}

[GlobalParams]
  gravity_vector = '0 0 0'
[]

[FluidProperties]
  [fp_air]
    type = IdealGasFluidProperties
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
    fp = fp_air
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 1
    A = ${A}
    closures = thm
    initial_vel = 0.003
    initial_p = 1e5
    initial_T = 300
  []

  [left_wall]
    type = SolidWall1Phase
    input = 'pipe:in'
  []
  [right_wall]
    type = SolidWall1Phase
    input = 'pipe:out'
  []
  [ht1]
    type = HeatTransferFromHeatFlux1Phase
    flow_channel = 'pipe'
    q_wall = 1000
    P_hf = ${P_hf}
  []
  [ht2]
    type = HeatTransferFromHeatFlux1Phase
    flow_channel = 'pipe'
    q_wall = 2000
    P_hf = ${P_hf}
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
  dt = 0.1
[]
