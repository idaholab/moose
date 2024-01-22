[Mesh]
  inactive = 'refine_left_and_top'
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
  [lower_left]
    type = LowerDBlockFromSidesetGenerator
    input = gen
    sidesets = 'left'
    new_block_name = 'lower_left'
  []
  [lower_right]
    type = LowerDBlockFromSidesetGenerator
    input = lower_left
    sidesets = 'right'
    new_block_name = 'lower_right'
  []
  [lower_top]
    type = LowerDBlockFromSidesetGenerator
    input = lower_right
    sidesets = 'top'
    new_block_name = 'lower_top'
  []
  [lower_bottom]
    type = LowerDBlockFromSidesetGenerator
    input = lower_top
    sidesets = 'bottom'
    new_block_name = 'lower_bottom'
  []
  [refine_left_and_top]
    type = RefineSidesetGenerator
    boundaries = 'left top'
    refinement = '1    1'
    boundary_side = 'primary primary'
    input = lower_bottom
  []
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [lower_constant]
    family = MONOMIAL
    order = CONSTANT
    block = 'lower_top lower_right lower_bottom lower_left'
  []
  [lower_first]
    family = MONOMIAL
    order = FIRST
    block = 'lower_top lower_right lower_bottom lower_left'
  []
  [higher]
    family = MONOMIAL
    order = CONSTANT
    block = '0'
  []
[]

[AuxKernels]
  [lower_constant]
    type = MaterialRealAux
    property = 'prop'
    variable = lower_constant
    boundary = 'top bottom right left'
  []
  [lower_first]
    type = MaterialRealAux
    property = 'prop'
    variable = lower_first
    boundary = 'top bottom right left'
  []
  [higher]
    type = MaterialRealAux
    property = 'prop'
    variable = higher
    block = 0
  []
[]

[Functions]
  [func]
    type = ParsedFunction
    expression = 'x + y'
  []
[]

[Materials]
  [func]
    type = GenericFunctionMaterial
    prop_names = 'prop'
    prop_values = 'func'
  []
[]

[Outputs]
  csv = true
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [avg_lower_constant_left]
    type = ElementAverageValue
    variable = lower_constant
    block = lower_left
  []
  [avg_lower_first_left]
    type = ElementAverageValue
    variable = lower_first
    block = lower_left
  []
  [avg_lower_constant_bottom]
    type = ElementAverageValue
    variable = lower_constant
    block = lower_bottom
  []
  [avg_lower_first_bottom]
    type = ElementAverageValue
    variable = lower_first
    block = lower_bottom
  []
  [avg_lower_constant_top]
    type = ElementAverageValue
    variable = lower_constant
    block = lower_top
  []
  [avg_lower_first_top]
    type = ElementAverageValue
    variable = lower_first
    block = lower_top
  []
  [avg_lower_constant_right]
    type = ElementAverageValue
    variable = lower_constant
    block = lower_right
  []
  [avg_lower_first_right]
    type = ElementAverageValue
    variable = lower_first
    block = lower_right
  []
[]
