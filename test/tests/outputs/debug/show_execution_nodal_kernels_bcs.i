[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  [sub]
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
    input = 'gen'
    block_id = '1'
  []
[]

[Debug]
  show_execution_order = ALWAYS
[]

[Variables]
  [u]
    block = '0 1'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [rxn]
    type = Reaction
    variable = u
  []
[]

[NodalKernels]
  [source]
    type = UserForcingFunctionNodalKernel
    variable = u
    block = '1'
    function = '1'
  []
  [bc_all]
    type = PenaltyDirichletNodalKernel
    variable = u
    value = 0
    boundary = 'right bottom'
    penalty = 1e10
  []
[]

[Executioner]
  type = Steady
[]
