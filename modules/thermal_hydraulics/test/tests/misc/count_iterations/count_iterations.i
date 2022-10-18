# This tests the "Debug/count_iterations" parameter, which creates
# post-processors for numbers of linear and nonlinear iterations. A dummy
# diffusion solve is performed, and the numbers of iterations are stored in a
# CSV file.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  coord_type = RZ
  rz_coord_axis = X
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [time_derivative]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Transient
  scheme = implicit-euler

  [TimeStepper]
    type = ConstantDT
    dt = 0.01
  []

  start_time = 0.0
  num_steps = 2
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = true
[]

[Debug]
  count_iterations = true
[]
