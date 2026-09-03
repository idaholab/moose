reg_kernel = DisplacementRegularization

[Mesh]
  type = GeneratedMesh
  dim = 1
  elem_type = EDGE3
  xmin = 0
  xmax = 1
  nx = 2
[]

[Variables]
  [u]
    order = SECOND
    family = LAGRANGE
  []
[]

[Kernels]
  [regularization]
    type = ${reg_kernel}
    variable = u
    regularization = huhu_lulu
    coefficient = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  csv = false
[]
