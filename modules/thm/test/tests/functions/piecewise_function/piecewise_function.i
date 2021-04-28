# This test tests the PiecewiseFunction, which pieces functions together.
# Piecing together the 2 CosineTransitionFunction functions should yield the
# CosineHumpFunction function. This test samples the PiecewiseFunction and the
# CosineHumpFunction and compares the samples using the
# VectorPostprocessorComparison post-processor.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Functions]
  [function_left]
    type = CosineTransitionFunction
    axis = y
    transition_center = 2
    transition_width = 2
    function1 = 5
    function2 = 20
  []
  [function_right]
    type = CosineTransitionFunction
    axis = y
    transition_center = 4
    transition_width = 2
    function1 = 20
    function2 = 5
  []
  [function_end]
    type = ConstantFunction
    value = 5
  []
  [function_piecewise]
    type = PiecewiseFunction
    axis = y
    axis_coordinates = '3 5'
    functions = 'function_left function_right function_end'
  []
  [function_gold]
    type = CosineHumpFunction
    axis = y
    hump_center_position = 3
    hump_width = 4
    hump_begin_value = 5
    hump_center_value = 20
  []
[]

[VectorPostprocessors]
  [piecewise_function_vpp]
    type = LineFunctionSampler
    functions = 'function_piecewise function_gold'
    sort_by = y
    start_point = '0 0 0'
    end_point = '0 6 0'
    num_points = 20
    execute_on = 'initial'
  []
[]

[Postprocessors]
  [matches_gold]
    type = VectorPostprocessorComparison
    comparison_type = equals
    vectorpostprocessor_a = piecewise_function_vpp
    vectorpostprocessor_b = piecewise_function_vpp
    vector_name_a = function_piecewise
    vector_name_b = function_gold
    execute_on = 'initial'
  []
[]

[Outputs]
  csv = true
  show = 'matches_gold'
  execute_on = 'initial'
[]
