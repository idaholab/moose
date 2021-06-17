
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Variables]
  [u]
    initial_condition = 1
  []
[]

[AuxVariables]
  [v]
    initial_condition = 10
  []
[]

[Kernels]
  [u_dot]
    type = TimeDerivative
    variable = u
  []
  [value_transfer]
    type = PorousFlowHeatMassTransfer
    variable = u
    v = v
    transfer_coefficient = 1e-1
  []
[]

[Postprocessors]
  [point_value]
    type = PointValue
    variable = u
    point = '0.5 0.5 0.'
    execute_on = 'initial timestep_end'
  []
[]

[Preconditioning]
  [basic]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 11
  dt = 1
[]


[Outputs]
  csv = true
[]
