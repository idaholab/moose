[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./l]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  [../]
[]

[Executioner]
  type = Steady
[]

[UserObjects]
  [./prime_product]
    type = PrimeProductUserObject
    execute_on = timestep_end
  [../]
[]

[Postprocessors]
  [./product]
    type = PrimeProductPostprocessor
    prime_product = prime_product
  [../]
[]

[Outputs]
  csv = true
[]
