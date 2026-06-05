[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 50
  []
[]

[Variables]
  [u]
  []
  [v]
    family = SCALAR
    initial_condition = 1
  []
[]

[Kernels]
  [u_diff]
    type = ADCoupledScalarDiffusion
    variable = u
    v = v
  []
  [u_rxn]
    type = ADLambdaU2
    variable = u
    lambda = 0
  []
[]

[ScalarKernels]
  [v_offdiag]
    type = ADCoupledFieldScalar
    variable = v
    u = u
  []
  [v_diag]
    type = ADScalarReaction
    variable = v
  []
  [v_exp_diag]
    type = ADExpU
    variable = v
  []
[]

[BCs]
  [left]
    type = ADDirichletBC
    variable = u
    value = 0
    boundary = 'left'
  []
  [right]
    type = ADDirichletBC
    variable = u
    value = 1
    boundary = 'right'
  []
[]

[Convergence]
  [u]
    type = DefaultNonlinearConvergence
    nl_abs_tol = 1e-10
    nl_rel_tol = 0
  []
  [v]
    type = DefaultNonlinearConvergence
    nl_abs_tol = 1e-10
    nl_rel_tol = 0
  []
[]
