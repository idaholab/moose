[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
    subdomain_name = left
  []
  [right_block]
    type = ParsedSubdomainMeshGenerator
    input = gen
    combinatorial_geometry = 'x > 0.5'
    block_id = 1
    block_name = right
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Materials]
  [coeff_mat]
    type = ADPiecewiseByBlockFunctorMaterial
    prop_name = coeff
    subdomain_to_prop_value = 'left 1'
  []
[]

[Postprocessors]
  [integral]
    type = ADElementIntegralFunctorPostprocessor
    block = right
    functor = coeff
    execute_on = INITIAL
  []
[]
