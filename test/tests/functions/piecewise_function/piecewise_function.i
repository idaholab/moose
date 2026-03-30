# This test tests the PiecewiseFunction, which pieces functions together.
# We piece together three arbitrary functions that are continuous from x = [0,5]
# and then compare to a VectorPostprocessor line sample of the function.

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 5
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Functions]
  [fn_1]
    type = ParsedFunction
    expression = 'x^2'
  []
  [fn_2]
    type = ParsedFunction
    expression = '4+sin(x-2)'
  []
  [fn_3]
    type = ParsedFunction
    expression = '4+sin(2)+(x-4)^3'
  []
  [function_piecewise]
    type = PiecewiseFunction
    axis = x
    axis_coordinates = '2 4'
    functions = 'fn_1 fn_2 fn_3'
  []
[]

[VectorPostprocessors]
  [vpp]
    type = LineFunctionSampler
    functions = 'function_piecewise'
    start_point = '0 0 0'
    end_point = '5 0 0'
    num_points = 26
    sort_by = x
    execute_on = 'INITIAL'
  []
[]

[Outputs]
  csv = true
  execute_on = 'initial'
[]
