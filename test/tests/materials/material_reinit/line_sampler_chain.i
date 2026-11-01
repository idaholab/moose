# A line material sampler reads a property produced by a material that reads a property from another
# material. Both producers must be computed.
[Mesh]
  type = GeneratedMesh
  # line material samplers do not support distributed meshes
  parallel_type = replicated
  dim = 1
  nx = 4
[]

[Materials]
  # sampled = 1 + A = 3, only consumed through the sampler, and A only through sampled
  [A]
    type = MaterialReinitTest
    property = A
    value = 2
  []
  [sampled]
    type = MaterialReinitTest
    property = sampled
    value = 1
    coupled_property = A
  []
[]

[VectorPostprocessors]
  [sampler]
    type = LineMaterialRealSampler
    start = '0 0 0'
    end = '1 0 0'
    property = sampled
    sort_by = x
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
