# Regression test for the execute_on schedule of TorchScriptProfileFunctorMaterial.
#
# The model inputs come from time-varying postprocessors (a_in = 10 + t and
# b_in = 5 - t). With execute_on = timestep_begin the material re-runs inference
# at the start of every step, so the published profiles track the inputs instead
# of staying frozen at their initial-setup values.
#
# profile_direction is perpendicular to the 1D mesh, so the profile coordinate
# s = 0 at every point and each functor reduces to its station-0 value (the raw
# model input a or b). The sampled value is therefore independent of position,
# and the reporters below equal the model inputs exactly.
#
# timestepSetup() runs before the timestep-begin postprocessors are recomputed
# (TransientBase::takeStep advances time, calls timestepSetup(), then
# onTimestepBegin()), so each step rebuilds the profiles from the input value as
# it stood at the *start* of the step -- a one-step visibility lag. The gold
# therefore shows profile_a_value = 10 then 11 (not 11 then 12) across the steps
# at t = 1 and t = 2.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 4
  xmin = 2
  xmax = 6
[]

[Functions]
  [a_func]
    type = ParsedFunction
    expression = '10 + t'
  []
  [b_func]
    type = ParsedFunction
    expression = '5 - t'
  []
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

    input_names = 'a_in b_in'

    profile_names = 'profile_a profile_b'
    profile_coordinates = '0 1 2'

    profile_origin = '0 0 0'
    profile_direction = '0 1 0'

    tensor_dtype = float64

    execute_on = 'timestep_begin'
  []
[]

[Postprocessors]
  [a_in]
    type = FunctionValuePostprocessor
    function = a_func
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    outputs = none
  []
  [b_in]
    type = FunctionValuePostprocessor
    function = b_func
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    outputs = none
  []

  [profile_a_value]
    type = ADElementExtremeFunctorValue
    functor = profile_a
    value_type = max
    execute_on = TIMESTEP_END
  []
  [profile_b_value]
    type = ADElementExtremeFunctorValue
    functor = profile_b
    value_type = max
    execute_on = TIMESTEP_END
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  start_time = 0
  dt = 1
  num_steps = 2
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
