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
  # cant use nodal variables because of the two blocks, which material to use
  # there is undefined
  [mat_x]
    family = MONOMIAL
    order = CONSTANT
  []
  [mat_y]
    family = MONOMIAL
    order = CONSTANT
  []
  [mat_z]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [matprop_to_aux_x]
    type = FunctorVectorElementalAux
    variable = mat_x
    functor = 'matprop'
    component = '0'
  []
  [matprop_to_aux_y]
    type = FunctorVectorElementalAux
    variable = mat_y
    functor = 'matprop'
    component = '1'
  []
  [matprop_to_aux_z]
    type = FunctorVectorElementalAux
    variable = mat_z
    functor = 'matprop'
    component = '2'
  []
[]

[Materials]
  [block0]
    type = GenericVectorFunctorMaterial
    block = '0'
    prop_names = 'matprop'
    prop_values = '4 2 1'
  []
  [block1]
    type = GenericVectorFunctorMaterial
    block = '1'
    prop_names = 'matprop'
    prop_values = 'f_x f_x f_z'
  []
[]

[Functions]
  [f_x]
    type = ParsedFunction
    expression = 'x + 2 * y'
  []
  [f_z]
    type = ParsedFunction
    expression = 'x * y - 2'
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
