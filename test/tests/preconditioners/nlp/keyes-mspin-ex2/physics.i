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
  [v_offdiag]
    type = ADCoupledFieldScalar
    variable = u
    scalar_variable = v
  []
[]

[ScalarKernels]
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
