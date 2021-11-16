[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
    xmax = 2
    ymax = 1
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1.0 0 0'
    block_id = 1
    top_right = '2.0 1.0 0'
  []
[]

[AuxVariables]
  [mat_x]
  []
  [mat_y]
  []
  [mat_z]
  []
[]

[AuxKernels]
  [matprop_to_aux_x]
    type = FunctorADVectorMatPropElementalAux
    variable = mat_x
    mat_prop = 'matprop'
    component = '0'
  []
  [matprop_to_aux_y]
    type = FunctorADVectorMatPropElementalAux
    variable = mat_y
    mat_prop = 'matprop'
    component = '1'
  []
  [matprop_to_aux_z]
    type = FunctorADVectorMatPropElementalAux
    variable = mat_z
    mat_prop = 'matprop'
    component = '2'
  []
[]

[Materials]
  [block0]
    type = ADGenericConstantVectorFunctorMaterial
    block = '0'
    prop_names = 'matprop'
    prop_values = '4 2 1'
  []
  [block1]
    type = ADGenericFunctionVectorFunctorMaterial
    block = '1'
    prop_names = 'matprop'
    prop_values = 'f_x f_x f_z'
  []
[]

[Functions]
  [f_x]
    type = ParsedFunction
    value = 'x + 2 * y'
  []
  [f_z]
    type = ParsedFunction
    value = 'x * y - 2'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
