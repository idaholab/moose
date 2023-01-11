[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = -0.01
  xmax = 0.01
[]

[Functions]
  [./fn]
    type = ParsedFunction
    expression = 'if(a < 0.8625, 1, 0)'
    symbol_names = 'a'
    symbol_values = 'a_avg'
  [../]
[]

[Variables]
  [./u]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxVariables]
  [./a]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[Kernels]
  [./td]
    type = TimeDerivative
    variable = u
  [../]
  [./rhs]
    type = BodyForce
    variable = u
    function = 1
  [../]
[]

[Postprocessors]
  [./fn_pps]
    type = FunctionValuePostprocessor
    function = fn
    execute_on = 'initial timestep_end'
  [../]

  [./a_avg]
    type = ElementAverageValue
    variable = a
  [../]
[]

[UserObjects]
  [./fn_uo]
    type = PostprocessorSpatialUserObject
    postprocessor = fn_pps
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.1
[]
