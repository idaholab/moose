[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 20
    ny = 20
    nz = 20
  []
  [image]
    type = ImageSubdomainGenerator
    input = gen
    file_base = stack/test
    file_suffix = png
    threshold = 6e4
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
