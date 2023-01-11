[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 8
[]

[Variables]
  [u][]
[]

[Kernels]
  [time]
    type = ADTimeDerivative
    variable = u
  []
  [diff]
    type = ADDiffusion
    variable = u
  []
  [force]
    type = BodyForce
    variable = u
    function = force
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = 't^3*x*y'
  []
  [force]
    type = ParsedFunction
    expression = '3*x*y*t^2'
  []
[]

[BCs]
  [all]
    type = FunctionDirichletBC
    variable = u
    function = exact
    boundary = 'left right top bottom'
  []
[]

[Postprocessors]
  [error]
    type = ElementL2Error
    function = exact
    variable = u
  []
  [h]
    type = AverageElementSize
  []
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 3
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  csv = true
[]
