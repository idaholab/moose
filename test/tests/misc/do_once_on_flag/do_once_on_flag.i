[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables/u]
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = BodyForce
    variable = u
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [initial]
    type = TestMooseDoOnceOnFlag
    do_once_on = INITIAL
  []
  [final]
    type = TestMooseDoOnceOnFlag
    do_once_on = FINAL
  []
  [linear]
    type = TestMooseDoOnceOnFlag
    do_once_on = LINEAR
  []
  [nonlinear]
    type = TestMooseDoOnceOnFlag
    do_once_on = NONLINEAR
  []
  [timestep_begin]
    type = TestMooseDoOnceOnFlag
    do_once_on = TIMESTEP_BEGIN
  []
  [timestep_end]
    type = TestMooseDoOnceOnFlag
    do_once_on = TIMESTEP_END
  []
[]
