[Mesh]
  [cyl2d_iga]
    type = FileMeshGenerator
    file = test_quadratic.e
    discontinuous_spline_extraction = true
  []
  allow_renumbering = false
[]

[Variables]
  [u]
    order = SECOND  # Must match mesh order
    family = RATIONAL_BERNSTEIN
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
    block = 0  # Avoid direct calculations on spline nodes
  []
  [null]
    type = NullKernel
    variable = u
    block = 1  # Keep kernel coverage check happy
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1.0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 2.0
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
