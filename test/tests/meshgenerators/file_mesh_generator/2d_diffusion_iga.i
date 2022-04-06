[Mesh]
  [cyl2d_iga]
    type = FileMeshGenerator
    file = PressurizedCyl_Patch6_4Elem.e
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
  [time]
    type = TimeDerivative
    variable = u
    block = 0  # Avoid direct calculations on spline nodes
  []
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

[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = 'sin(x)'
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
  solve_type = NEWTON
  dtmin = 1
[]

[Outputs]
  vtk = true
[]
