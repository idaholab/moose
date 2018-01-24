[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Adaptivity]
  [./Indicators]
    [./error]
      type = ValueJumpIndicator
      variable = something
    [../]
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./leftright]
    type = BoundingBoxIC
    variable = something
    inside = 1
    y2 = 1
    y1 = 0
    x2 = 0.5
    x1 = 0
  [../]
[]

[AuxVariables]
  [./something]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 1
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
[]

[Outputs]
  exodus = true
[]
