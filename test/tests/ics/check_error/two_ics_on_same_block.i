[Mesh]
  type = FileMesh
  file = '../../kernels/block_kernel/rectangle.e'
[]

[Variables]
  [./u]
  [../]
[]

[ICs]
  [./block]
    type = ConstantIC
    variable = u
    block = 1
    value = 0.5
  [../]
  [./block2]
    type = ConstantIC
    variable = u
    block = 1
    value = 2
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

[Outputs]
[]
