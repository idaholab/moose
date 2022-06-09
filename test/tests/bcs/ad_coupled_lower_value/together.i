[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
  [lower_d]
    type = LowerDBlockFromSidesetGenerator
    input = square
    new_block_name = 'lower'
    sidesets = 'top right'
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [u]
    block = 0
  []
[]

[AuxVariables]
  [lower]
    block = 'lower'
    initial_condition = 10
  []
[]

[Kernels]
  [diff]
    type = ADDiffusion
    variable = u
    block = 0
  []
[]

[BCs]
  [dirichlet]
    type = ADDirichletBC
    variable = u
    boundary = 'left'
    value = 0
  []
  [neumann]
    type = ADCoupledLowerValue
    variable = u
    boundary = 'right'
    lower_d_var = lower
  []
[]

[Executioner]
  type = Steady
  residual_and_jacobian_together = true
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
