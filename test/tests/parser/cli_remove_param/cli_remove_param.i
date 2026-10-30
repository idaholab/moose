[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2                     # Removed via --remove-param in some tests
    ny = 2
    dim = 2
  []
[]

[Variables]
  active = 'u'

  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  active = 'diff'

  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  active = 'left right'

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

  solve_type = 'PJFNK'
[]

[Postprocessors]
  [num_elems]
    type = NumElements
    execute_on = 'initial timestep_end'
  []
  [extra]
    type = NumElements
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
[]
