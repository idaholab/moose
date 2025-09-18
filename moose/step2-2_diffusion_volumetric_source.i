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
    type = Diffusion
    variable = u
  []
  [source]
    type = BodyForce
    variable = u
    function = '1e9 * sqrt(x^2 + y^2)'
  []
[]

[BCs]
  [inner_dirichlet]
    type = DirichletBC
    variable = u
    boundary = inner
    value = 0
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
