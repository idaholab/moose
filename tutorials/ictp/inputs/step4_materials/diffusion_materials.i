[Mesh/fuel_pin]
  type = FileMeshGenerator
  file = ../step1_input_and_meshing/fuel_pin_in.e
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
    boundary = water_solid_interface
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
