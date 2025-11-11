###########################################################
# This is a simple test of the CoupledTimeDerivative kernel.
# The expected solution for the variable v is
# v(x) = 1/2 * (x^2 + x)
###########################################################

[Mesh]
  type = GeneratedMesh
  nx = 5
  ny = 5
  dim = 2
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[Kernels]
  [time_u]
    type = KokkosTimeDerivative
    variable = u
  []
  [fn_u]
    type = KokkosBodyForce
    variable = u
  []
  [time_v]
    type = KokkosCoupledTimeDerivative
    variable = v
    v = u
  []
  [diff_v]
    type = KokkosDiffusion
    variable = v
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    variable = v
    boundary = 'left'
    value = 0
  []
  [right]
    type = KokkosDirichletBC
    variable = v
    boundary = 'right'
    value = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
