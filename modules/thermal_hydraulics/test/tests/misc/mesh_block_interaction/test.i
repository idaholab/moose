[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[Components]
  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'iwst_pipe_1:in'
    m_dot = 1
    T = 100
  []

  [iwst_pipe_1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '0 1 0'
    length = 10
    n_elems = 15
    A = 1
    D_h = 1
  []

  [outlet]
    type = Outlet1Phase
    input = 'iwst_pipe_1:out'
    p = 1e5
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 10
  abort_on_solve_fail = true
[]
