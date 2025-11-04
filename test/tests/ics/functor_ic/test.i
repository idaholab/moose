[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 1
    ix = 1
    dx = 1
  []
  allow_renumbering = false
[]

[Problem]
  solve = false
[]

[Variables]
  [u]
  []
[]

[ICs]
  [u_init]
    type = FunctorIC
    variable = 'u'
    functor = 'pp_functor'
  []
[]

# Potential objects defining functor_v
[Postprocessors]
  [pp_functor]
    type = FunctionValuePostprocessor
    function = 'linear'
    force_preic = true
  []
[]

[Functions]
  [linear]
    type = ParsedFunction
    expression = '2 + t'
  []
  [function_functor]
    type = ParsedFunction
    expression = '3'
  []
[]

[AuxVariables]
  [variable_functor]
  []
[]

[FunctorMaterials]
  [fmat]
    type = GenericFunctorMaterial
    prop_names = 'mat_functor'
    prop_values = '4'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [check_var_value]
    type = ElementalVariableValue
    variable = 'u'
    elementid = 0
  []
[]
