[Mesh/fuel_pin]
  type = FileMeshGenerator
  input = fuel_pin.e
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [inner_dirichlet]
    type = DirichletBC
    variable = u
    boundary = inner
    value = 0
  []
  [outer_dirichlet]
    type = DirichletBC
    variable = u
    boundary = outer
    value = 1
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
