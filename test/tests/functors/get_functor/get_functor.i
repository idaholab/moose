[GlobalParams]
  execute_on = 'INITIAL'
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 5
[]

[AuxVariables]
  [testvar]
  []
  [testppdot]
    family = MONOMIAL
    order = CONSTANT
  []
  [testppdiv]
  []
[]

[AuxKernels]
  [testvar_auxkern]
    type = FunctionAux
    variable = testvar
    function = testvar_fn
    execute_on = 'INITIAL'
  []
  [testppdot_auxkern]
    type = TimeDerivativeAux
    variable = testppdot
    functor = testpp
  []
  [testppdiv_auxkern]
    type = DivergenceAux
    variable = testppdiv
    u = testpp
    v = testpp
    w = testpp
  []
[]

[FunctorMaterials]
  [testfmat]
    type = GenericFunctorMaterial
    prop_names = 'testfmprop'
    prop_values = 'testfmat_fn'
  []
[]

[Functions]
  [testvar_fn]
    type = ParsedFunction
    expression = '10*x'
  []
  [testfmat_fn]
    type = ParsedFunction
    expression = '50*x'
  []
  [testfn]
    type = ParsedFunction
    expression = '25*x'
  []
[]

[Postprocessors]
  [testpp]
    type = ConstantPostprocessor
    value = 2
  []

  [get_var]
    type = ElementIntegralFunctorPostprocessor
    functor = testvar
  []
  [get_fn]
    type = ElementExtremeFunctorValue
    functor = testfn
    value_type = max
  []
  [get_fmprop]
    type = ElementExtremeFunctorValue
    functor = testfmprop
    value_type = max
  []
  [get_pp]
    type = ElementExtremeFunctorValue
    functor = testpp
    value_type = max
    execution_order_group = 1
  []
  [get_ppdiv]
    type = ElementAverageValue
    variable = testppdiv
  []
  [get_ppdot]
    type = ElementAverageValue
    variable = testppdot
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
[]
