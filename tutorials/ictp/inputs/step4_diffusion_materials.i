[Mesh/fuel_pin]
  type = FileMeshGenerator
  file = fuel_pin.e
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diffusion]
    type = MatDiffusion
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

[Materials]
  [D_fuel]
    type = GenericConstantMaterial
    prop_names = D
    prop_values = 100
    block = fuel
  []
  [D_clad]
    type = GenericConstantMaterial
    prop_names = D
    prop_values = 2
    block = clad
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
