[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Functions]
  [initial_value]
    type = ParsedFunction
    expression = 'x'
  []
[]

[Variables]
  [u]
    [InitialCondition]
      type = FunctionIC
      function = initial_value
    []
  []
[]

[Kernels]
  [diff]
    type = KokkosDiffusion
    variable = u
  []
  [ie]
    type = KokkosTimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = KokkosSinDirichletBC
    variable = u
    boundary = left
    initial = 0.0
    final = 1.0
    duration = 10.0
  []
  [right]
    type = KokkosSinDirichletBC
    variable = u
    boundary = right
    initial = 1.0
    final = 0.0
    duration = 10.0
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 10
  dt = 1.0
[]

[Outputs]
  exodus = true
[]
