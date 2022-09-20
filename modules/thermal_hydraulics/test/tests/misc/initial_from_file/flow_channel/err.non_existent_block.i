[GlobalParams]
  closures = simple_closures

  initial_from_file = 'steady_state_out.e'
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
    k = 0.5
    mu = 281.8e-6
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [asdf]
    type = FlowChannel1Phase
    fp = fp
    # geometry
    position = '1 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 3
    A = 1.907720E-04
    D_h = 1.698566E-02
    f = 0.1
  []
  [inlet]
    type = SolidWall1Phase
    input = 'asdf:in'
  []
  [outlet]
    type = SolidWall1Phase
    input = 'asdf:out'
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
[]
