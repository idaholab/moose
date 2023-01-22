[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  nx = 10
  ymin = 0
  ymax = 2
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./v1]
    initial_condition = 3
  [../]

  [./v2]
  [../]
[]

[ICs]
  [./v2_ic]
    type = FunctionIC
    variable = v2
    function = v2_func
  [../]
[]

[Functions]
  [./v2_func]
    type = ParsedFunction
    expression = 'x + 2 * y'
  [../]

  [./reference]
    type = ParsedFunction
    expression = '3 * (-1) * 3.5 + (x + 2 * y) * 15 * 1.2'
  [../]
[]

[Materials]
  [./mat]
    type = GenericConstantMaterial
    prop_names = 'm1 m2'
    prop_values = '-1 15'
  [../]
[]

[Kernels]
  [./reaction]
    type = Reaction
    variable = u
  [../]

  [./coupled]
    type = MatCoupledForce
    variable = u
    v = 'v1 v2'
    coef = '3.5 1.2'
    material_properties = 'm1 m2'
  [../]
[]

[Postprocessors]
  [./error]
    type = ElementL2Error
    function = reference
    variable = u
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
