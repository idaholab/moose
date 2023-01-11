[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [u]
  []

  [n]
    family = SCALAR
    order = FIRST
  []
[]

[ICs]
  [n_ic]
    type = ScalarConstantIC
    variable = n
    value = 0
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  []
[]

[ScalarKernels]
  [ctd]
    type = ODECoefTimeDerivative
    variable = n
    coef = 2.
  []
  [ode1]
    type = ParsedODEKernel
    variable = n
    expression = '-4'
  []
[]

[BCs]
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
  abort_on_solve_fail = true
[]

[Outputs]
  csv = true
[]
