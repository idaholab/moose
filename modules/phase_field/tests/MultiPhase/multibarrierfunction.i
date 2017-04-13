[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
  nz = 0
  xmin = 0
  xmax = 20
  ymin = 0
  ymax = 20
  elem_type = QUAD4
[]

[AuxVariables]
  [./eta1]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 7.0
      y1 = 10.0
      radius = 5.0
      invalue = 0.9
      outvalue = 0.1
      int_width = 2.0
    [../]
  [../]
  [./eta2]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 13.0
      y1 = 10.0
      radius = 5.0
      invalue = 0.9
      outvalue = 0.1
      int_width = 2.0
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
  [./multibarrier]
    type = MultiBarrierFunctionMaterial
    etas = 'eta1 eta2'
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
