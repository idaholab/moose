# ExponentialDecay
# Note that we do not get u - ref = (u_0 - ref) * exp(-rate * t)
# because of the time discretisation.  We are solving
# the equation
# (u(t+dt) - u(t))/dt = -rate * (u(t+dt) - ref)
# which has solution
# u(t+dt) = (u(t) + rate * ref * dt) / (1 + rate * dt)
# With u(0)=2, rate=1.5, ref=1 and dt=0.2 we get
# u(0.2) = 1.769
# u(0.4) = 1.592
# u(0.6) = 1.455
# u(0.8) = 1.350
# u(1.0) = 1.269
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Variables]
  [u]
    initial_condition = 2
  []
[]

[Kernels]
  [time_derivative]
    type = TimeDerivative
    variable = u
  []
  [exp_decay]
    type = PorousFlowExponentialDecay
    variable = u
    rate = 1.5
    reference = 1.0
  []
[]

[Postprocessors]
  [u]
    type = PointValue
    variable = u
    point = '0 0 0'
  []
[]

[Preconditioning]
  [check]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 0.2
  end_time = 1
[]

[Outputs]
  csv = true
[]
