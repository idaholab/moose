[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 40
  ny = 10
  nz = 10
  allow_renumbering = false
[]

[Materials]
  [u_mat]
    type = GenericFunctorMaterial
    prop_names = 'u'
    prop_values = 'u_fn'
  []
[]

[AuxVariables]
  [u_layered_average]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [u_layered_average_kern]
    type = SpatialUserObjectAux
    variable = u_layered_average
    user_object = nplaf
    boundary = 'bottom top'
    execute_on = 'INITIAL'
  []
[]

[Functions]
  [u_fn]
    type = ParsedFunction
    expression = 'x + y + z'
  []
[]

[UserObjects]
  [nplaf]
    type = LayeredSideIntegralFunctor
    direction = x
    num_layers = 10
    functor = u
    boundary = 'bottom top'
    execute_on = 'INITIAL'
  []
[]

[VectorPostprocessors]
  [test_vpp]
    type = SideValueSampler
    variable = u_layered_average
    boundary = 'bottom top'
    sort_by = id
    execute_on = 'INITIAL'
  []
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
