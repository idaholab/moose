[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Functions]
  [rxn_func]
    type = ParsedFunction
    value = 'log(3)*log(3)'
  []
[]

[Kernels]
  [diff]
    type = ADDiffusion
    variable = u
  []
  [reaction]
    type = ADFuncReaction
    variable = u
    func = rxn_func
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 3
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
