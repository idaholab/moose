#stress function fcn_00 showing only the stress field within the box is being used to average the stress.

[Mesh]
  file = crack_loop.e
[]

[Problem]
  solve = false
[]

[Functions]
  [fcn_00]
    type = ParsedFunction
    expression = 'if(y>0,10+abs(x),10)'
  []
  [fcn]
    type = ParsedFunction
    expression = '100'
  []
[]

[Materials]
  [tensor]
    type = GenericFunctionRankTwoTensor
    tensor_name = generic_stress
    # tensor values are column major-ordered
    tensor_functions = 'fcn_00 fcn fcn  fcn fcn fcn  fcn fcn fcn'
    outputs = all
  []
  [scalar]
    type = GenericFunctionMaterial
    prop_names = scalar_kcrit
    prop_values = fcn_00
  []
[]

[UserObjects]
  [crack]
    type = CrackFrontDefinition
    crack_direction_method = CurvedCrackFront
    boundary = 1001
  []
[]

[VectorPostprocessors]
  [CrackFrontNonlocalStress]
    type = CrackFrontNonlocalStress
    stress_name = stress
    base_name = generic
    crack_front_definition = crack
    box_length = 0.1
    box_width = 0.1
    box_height = 0.05
  []
  [CrackFrontNonlocalKcrit]
    type = CrackFrontNonlocalScalarMaterial
    property_name = kcrit
    base_name = scalar
    crack_front_definition = crack
    box_length = 0.1
    box_width = 0.1
    box_height = 0.05
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
