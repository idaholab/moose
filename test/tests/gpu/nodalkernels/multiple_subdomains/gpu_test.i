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

[GPUKernels]
  [diff]
    type = GPUDiffusion
    variable = u
  []
  [rxn]
    type = GPUReaction
    variable = u
  []
[]

[GPUNodalKernels]
  [source]
    type = GPUConstantRate
    variable = u
    block = '0 1'
    rate = 1
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
