[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [a]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = ADDiffusion
    variable = u
  []
  [force]
    type = ADCoupledForce
    variable = u
    v = a
  []
[]

[AuxKernels]
  [a]
    variable = a
    type = ConstantAux
    value = 10
  []
[]

[BCs]
  [left]
    type = ADDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = ADDirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]
