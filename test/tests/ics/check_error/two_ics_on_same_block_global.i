[Mesh]
  type = FileMesh
  file = 'rectangle.e'
[]

[Variables]
  [./u]
  [../]
[]

[ICs]
  [./block]
    type = ConstantIC
    variable = u
    value = 2
  [../]

  [./block2]
    type = ConstantIC
    variable = u
    value = 0.5
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[Executioner]
  type = Steady
[]
