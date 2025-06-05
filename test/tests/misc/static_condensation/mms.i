[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
  xmax = 1
  xmin = -1
  second_order = true
[]

[Variables]
  [u]
    order = SECOND
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [force]
    type = BodyForce
    variable = u
    function = 'forcing'
  []
[]

[BCs]
  [left]
    type = FunctionPenaltyDirichletBC
    variable = u
    boundary = left
    function = 'exact'
    penalty = 1e8
  []
  [right]
    type = FunctionPenaltyDirichletBC
    variable = u
    boundary = right
    function = 'exact'
    penalty = 1e8
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = 'sin(x)'
  []
  [forcing]
    type = ParsedFunction
    expression = 'sin(x)'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [h]
    type = AverageElementSize
  []
  [L2u]
    type = ElementL2Error
    variable = u
    function = 'exact'
  []
[]
