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
    boundary = water_solid_interface
    value = 1
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
