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
  [u_layered_integral]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [u_layered_integral_kern]
    type = SpatialUserObjectAux
    variable = u_layered_integral
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
    type = NearestPointLayeredSideIntegralFunctor
    direction = x
    points='
      0.25 0 0.25
      0.75 0 0.25
      0.25 0 0.75
      0.75 0 0.75'
    # Each layer has exactly 4 elements in the x direction. Note that to avoid inconsistent
    # results, we should always avoid aligning layer edges with element centroids.
    num_layers = 10
    functor = u
    boundary = 'bottom top'
    execute_on = 'INITIAL'
  []
[]

[VectorPostprocessors]
  [test_vpp]
    type = SideValueSampler
    variable = u_layered_integral
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
