[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Functions]
  [x_fn]
    type = ParsedFunction
    expression = x
  []
  [zero]
    type = ConstantFunction
    value = 0
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [flux]
    type = ADFluxDivergence
    variable = u
  []
  [reaction]
    type = ADReaction
    variable = u
  []
[]

[Materials]
  [flux]
    type = ADGenericFunctionVectorMaterial
    prop_names = 'flux'
    prop_values = 'x_fn zero zero'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Postprocessors]
  [u_avg]
    type = ElementAverageValue
    variable = u
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [csv]
    type = CSV
    file_base = ad_flux_divergence_out
  []
[]

