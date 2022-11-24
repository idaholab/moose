[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Problem]
  error_on_jacobian_nonzero_reallocation = true
[]

[Executioner]
  type = Steady
[]

[Testing]
  [./LotsOfDiffusion]
    [./vars]
      number = 1
      diffusion_coefficients = 1
    [../]
  [../]
[]
