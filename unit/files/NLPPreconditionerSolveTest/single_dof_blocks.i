[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 0
  []
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[NodalKernels]
  [uu]
    type = ADReactionNodalKernel
    variable = u
  []
  [uv]
    type = ADCoupledForceNodalKernel
    variable = u
    v = v
    coef = -1
  []
  [u_force]
    type = UserForcingFunctorNodalKernel
    functor = 2
    variable = u
  []
  [vu]
    type = ADCoupledForceNodalKernel
    v = u
    variable = v
    coef = 1
  []
  [vv]
    type = ADReactionNodalKernel
    variable = v
  []
[]

[Convergence]
  [nl0]
    type = DefaultNonlinearConvergence
    nl_abs_tol = 1e-12
  []
  [nl1]
    type = DefaultNonlinearConvergence
    nl_abs_tol = 1e-12
  []
[]

[Postprocessors]
  [u]
    type = ElementAverageValue
    variable = u
    execute_on = 'final'
  []
  [v]
    type = ElementAverageValue
    variable = v
    execute_on = 'final'
  []
[]

[Outputs]
  console = false
[]
