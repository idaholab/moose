[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  nz = 0
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  elem_type = QUAD4
[]

[AuxVariables]
  [./eta1]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      variable = eta1
      function = x
    [../]
  [../]
  [./eta2]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      variable = eta2
      function = 1-x
    [../]
  [../]
  [./eta3]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      variable = eta3
      function = y
    [../]
  [../]
[]

[BCs]
  [./Periodic]
    [./All]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  [./suppressionbarrier]
    type = ThirdPhaseSuppressionMaterial
    etas = 'eta1 eta2 eta3'
    function_name = g
    outputs = exodus
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
[]
