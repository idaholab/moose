[Mesh]
  [./cmg]
    type = CartesianMeshGenerator
    dim = 1
    dx = '1.0 1.0'
    ix = '1 1'
    subdomain_id = '0 1'
  [../]
[]

[Adaptivity]
  steps = 1
  marker = 'uniform'
  [./Markers/uniform]
    type = UniformMarker
    mark = refine
    block = 0
  [../]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [./max_h_level]
    type = ElementMaxHLevelPostProcessor
  [../]
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
