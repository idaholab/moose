[Mesh]
  second_order = true
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
  [block]
    type = ParsedSubdomainMeshGenerator
    input = square
    block_id = 1
    combinatorial_geometry = 'x > 0.5'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[AuxVariables]
  [nodal]
    [InitialCondition]
      type = FunctionIC
      function = '1 + x*x + y*y*y'
    []
  []
  [fe_higher_order]
    order = SECOND
    [InitialCondition]
      type = FunctionIC
      function = '1 + x*x + y*y*y'
    []
  []
  [fe_elemental]
    family = MONOMIAL
    order = SECOND
    [InitialCondition]
      type = FunctionIC
      function = '1 + x*x + y*y*y'
    []
  []
  [fv_var]
    type = MooseVariableFVReal
    [InitialCondition]
      type = FunctionIC
      function = '1 + x*x + y*y*y'
    []
  []
[]

[Functions]
  [f]
    type = ParsedFunction
    expression = '1 + x*x + y*y*y'
  []
[]

[Materials]
  [two_piece]
    type = ADPiecewiseByBlockFunctorMaterial
    prop_name = 'mat'
    subdomain_to_prop_value = '0 nodal 1 fv_var'
  []
[]

[Postprocessors]
  [fe]
    type = ADElementIntegralFunctorPostprocessor
    functor = nodal
  []
  [fe_higher_order]
    type = ADElementIntegralFunctorPostprocessor
    functor = fe_higher_order
  []
  [fe_elemental]
    type = ADElementIntegralFunctorPostprocessor
    functor = fe_elemental
  []
  [fv]
    type = ADElementIntegralFunctorPostprocessor
    functor = fv_var
  []
  [function]
    type = ElementIntegralFunctorPostprocessor
    functor = f
    prefactor = f
  []
  [functor_matprop]
    type = ADElementIntegralFunctorPostprocessor
    functor = mat
  []
[]

[Outputs]
  file_base = out
  exodus = false
  csv = true
[]
