[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
  []
  [sub]
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
    input = 'gen'
    block_id = '1'
  []
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
    block = '0 1'
    function = '1'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
