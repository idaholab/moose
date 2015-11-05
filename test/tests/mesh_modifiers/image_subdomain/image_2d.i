[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 176
  ny = 287
[]

[Variables]
  [./u]
  [../]
[]

[MeshModifiers]
  [./image]
    type = ImageSubdomain
    file = kitten.png #../../functions/image_function/stack/test
    threshold = 100
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
[../]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
