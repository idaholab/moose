[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
  [v]
    order = FIRST
    family = LAGRANGE
  []
  [w]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = KokkosDiffusion
    variables = 'u v w'
  []
  [bf]
    type = KokkosBodyForce
    variables = 'u v'
    postprocessor = ramp
  []
[]

[Functions]
  [ramp]
    type = ParsedFunction
    expression = 't'
  []
[]

[Postprocessors]
  [ramp]
    type = FunctionValuePostprocessor
    function = ramp
    execute_on = linear
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    variables = 'v w'
    boundary = 3
    value = 0
  []

  [right]
    type = KokkosDirichletBC
    variables = 'v w'
    boundary = 1
    value = 0
  []

  [top]
    type = KokkosDirichletBC
    variable = 'u'
    boundary = 2
    value = 0
  []

  [bottom]
    type = KokkosDirichletBC
    variable = 'u'
    boundary = 0
    value = 0
  []
[]

[Executioner]
  type = Transient
  dt = 1.0
  end_time = 1.0

  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
