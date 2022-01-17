# Tests the weighted average aux, which computes a weighted average of an
# arbitrary number of aux variables, using other aux variables as the weights.
# For this example, the values being averaged are
#   value1 = 4
#   value2 = 9
# and the weights are
#   weight1 = 2
#   weight2 = 3
# The result should then be
#   weighted_average = (weight1 * value1 + weight2 * value2) / (weight1 + weight2)
#                    = (2 * 4 + 3 * 9) / (2 + 3)
#                    = 35 / 5
#                    = 7

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  allow_renumbering = false
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[AuxVariables]
  [weighted_average]
    family = MONOMIAL
    order = CONSTANT
  []
  [value1]
    family = MONOMIAL
    order = CONSTANT
  []
  [value2]
    family = MONOMIAL
    order = CONSTANT
  []
  [weight1]
    family = MONOMIAL
    order = CONSTANT
  []
  [weight2]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [weighted_average_auxkernel]
    type = WeightedAverageAux
    variable = weighted_average
    values = 'value1 value2'
    weights = 'weight1 weight2'
  []
  [value1_kernel]
    type = ConstantAux
    variable = value1
    value = 4
  []
  [value2_kernel]
    type = ConstantAux
    variable = value2
    value = 9
  []
  [weight1_kernel]
    type = ConstantAux
    variable = weight1
    value = 2
  []
  [weight2_kernel]
    type = ConstantAux
    variable = weight2
    value = 3
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [weighted_average_pp]
    type = ElementalVariableValue
    elementid = 0
    variable = weighted_average
  []
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]
