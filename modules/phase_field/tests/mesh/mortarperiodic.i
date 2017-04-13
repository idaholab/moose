[Mesh]
  type = MortarPeriodicMesh
  dim = 3
  nx = 3
  ny = 2
  nz = 4
  xmin = 0.3
  xmax = 1.2
  ymin = 0.1
  ymax = 0.9
  zmin = 0.2
  zmax = 1.1
  periodic_directions = 'x y z'
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
