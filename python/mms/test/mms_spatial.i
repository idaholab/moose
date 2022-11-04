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
    expression = 'sin(2*pi*x)*sin(2*pi*y)'
  []
  [force]
    type = ParsedFunction
    expression = '8*pi^2*sin(2*x*pi)*sin(2*y*pi)'
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
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  csv = true
[]
