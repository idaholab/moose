[Mesh]
  [cyl2d_iga]
    type = FileMeshGenerator
    file = test_quadratic.e
    discontinuous_spline_extraction = true
  []
[]

[Variables]
  [u]
    order = SECOND  # Must match mesh order
    family = RATIONAL_BERNSTEIN
  []
[]

[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = 'x'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
    block = 0  # Avoid direct calculations on spline nodes
  []
  [./time]
    type = TimeDerivative
    variable = u
    block = 0
  [../]
  [null]
    type = NullKernel
    variable = u
    block = 1  # Keep kernel coverage check happy
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  solve_type = NEWTON
  dt = 1
[]

[Outputs]
  exodus = true
[]
