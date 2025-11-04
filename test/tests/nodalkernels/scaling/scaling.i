[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 40
[]

[Variables]
  [u][]
[]

[NodalKernels]
  [time]
    type = CoefTimeDerivativeNodalKernel
    variable = u
    coeff = 2
  []
  [reaction]
    type = ReactionNodalKernel
    variable = u
    coeff = 2
  []
  [ffn]
    type = UserForcingFunctorNodalKernel
    variable = u
    functor = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  automatic_scaling = true
  verbose = true
[]

[Outputs]
  exodus = true
[]
