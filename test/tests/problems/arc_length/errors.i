[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 4
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [load]
    type = BodyForce
    variable = u
    value = 1
  []
[]

[Problem]
  type = ArcLengthProblem
  step_size = 0.1
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]
