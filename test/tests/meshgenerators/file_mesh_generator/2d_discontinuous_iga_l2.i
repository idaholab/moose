[Mesh]
  [cyl2d_iga]
    type = FileMeshGenerator
    file = PressurizedCyl_Patch6_4Elem.e
    discontinuous_spline_extraction = true
  []
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
  [rxn]
    type = Reaction
    variable = u
    rate = -0.1
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
    function = '1.0'
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
  solve_type = NEWTON
  dtmin = 1
[]

[Outputs]
  exodus = true
[]
