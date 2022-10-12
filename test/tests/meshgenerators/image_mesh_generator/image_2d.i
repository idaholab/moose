[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 176
    ny = 287
  []
  [image]
    type = ImageSubdomainGenerator
    input = gen
    file = kitten.png #../../functions/image_function/stack/test
    threshold = 100
  []
[]

[Variables]
  [u]
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
