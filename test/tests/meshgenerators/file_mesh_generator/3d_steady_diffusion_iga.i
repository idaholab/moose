[Mesh]
  [cyl2d_iga]
    type = FileMeshGenerator
    file = Cube_With_Sidesets.e
  []
  allow_renumbering = false
  parallel_type = replicated
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
    boundary = 3
    value = 0
  []

  [right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  vtk = true
[]
