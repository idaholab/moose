[Mesh]
  type = SphereSurfaceMesh
  center = '1 2 3'
  radius = 4
  depth = 2
[]

[AuxVariables]
  [./ax]
    [./InitialCondition]
      type = FunctionIC
      function = x
    [../]
  [../]
  [./ay]
    [./InitialCondition]
      type = FunctionIC
      function = y
    [../]
  [../]
  [./az]
    [./InitialCondition]
      type = FunctionIC
      function = z
    [../]
  [../]
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  exodus = true
[]
