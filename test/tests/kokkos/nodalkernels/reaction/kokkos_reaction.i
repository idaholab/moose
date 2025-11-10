[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
  [nodal_ode]
  []
[]

[Kernels]
  [diff]
    type = KokkosCoefDiffusion
    variable = u
    coef = 0.1
  []
  [time]
    type = KokkosTimeDerivative
    variable = u
  []
[]

[NodalKernels]
  [td]
    type = KokkosTimeDerivativeNodalKernel
    variable = nodal_ode
  []
  [reaction]
    type = KokkosReactionNodalKernel
    variable = nodal_ode
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = KokkosDirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 20
  dt = 0.1
[]

[Outputs]
  exodus = true
[]
