[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 4
  xmin = 0
  xmax = 1
[]

[Functions]
  [./fn]
    type = PiecewiseConstant
    axis = x
    x = '0 0.25 0.50 0.75'
    y = '5 2 3 6'
  [../]
[]

[Materials]
  [./mat]
    type = GenericFunctionMaterial
    prop_names = 'mat_prop'
    prop_values = 'fn'
  [../]
[]

[Postprocessors]
  [./avg]
    type = ElementAverageMaterialProperty
    mat_prop = mat_prop
    execute_on = 'INITIAL'
  [../]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL'
[]
