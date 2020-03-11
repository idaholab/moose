[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 15
  ny = 15
  nz = 0
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Materials]
  #
  [./funcmat]
    type = GenericFunctionMaterial
    block = 0
    prop_names  = 'C'
    prop_values = 'x^2-y^2'
    outputs = exodus
  [../]
[]

[Kernels]
  [./value]
    type = MaterialPropertyValue
    prop_name = C
    variable = c
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]

