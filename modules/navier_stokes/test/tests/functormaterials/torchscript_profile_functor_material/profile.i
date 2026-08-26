[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 4
  xmin = 2
  xmax = 6
[]

[UserObjects]
  [profile_network]
    type = TorchScriptUserObject
    filename = two_profiles.pt
    load_during_construction = true
    execute_on = NONE
  []
[]

[FunctorMaterials]
  [profiles]
    type = TorchScriptProfileFunctorMaterial
    torch_script_userobject = profile_network

    input_values = '10 5'

    profile_names = 'profile_a profile_b'
    profile_coordinates = '0 1 2'

    profile_origin = '2 0 0'
    profile_direction = '2 0 0'

    coordinate_scale = 2
    out_of_range_behavior = error
    tensor_dtype = float64
  []
[]

[Postprocessors]
  [a_max]
    type = ADElementExtremeFunctorValue
    functor = profile_a
    value_type = max
    execute_on = INITIAL
  []

  [a_min]
    type = ADElementExtremeFunctorValue
    functor = profile_a
    value_type = min
    execute_on = INITIAL
  []

  [b_max]
    type = ADElementExtremeFunctorValue
    functor = profile_b
    value_type = max
    execute_on = INITIAL
  []

  [b_min]
    type = ADElementExtremeFunctorValue
    functor = profile_b
    value_type = min
    execute_on = INITIAL
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
  execute_on = INITIAL
[]
