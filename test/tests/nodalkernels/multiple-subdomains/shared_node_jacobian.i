# The nodal kernels give u - 1 = 0 at every node, with the nodes at x = 0.5 shared by blocks 0
# and 1. With an exact Jacobian, Newton converges in one iteration.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
  []
  [sub]
    type = SubdomainBoundingBoxGenerator
    input = gen
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
    block_id = 1
  []
[]

[Variables]
  [u]
  []
[]

[NodalKernels]
  [reaction]
    type = ReactionNodalKernel
    variable = u
  []
  [source]
    type = ConstantRate
    variable = u
    rate = 1
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]
