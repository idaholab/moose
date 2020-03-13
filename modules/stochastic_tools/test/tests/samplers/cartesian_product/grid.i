[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  ny = 1
[]

[Variables/u]
[]

[Samplers/sample]
  type = CartesianProduct
  linear_space_items = '10 1.5 3
                        20 1 4
                        130 10 2'
  execute_on = 'initial timestep_end'
[]

[VectorPostprocessors/data]
  type = SamplerData
  sampler = sample
  execute_on = 'initial timestep_end'
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
[]
