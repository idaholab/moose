[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmax = 5
    ymax = 5
  []
[]

[Functions/parsed_function]
  type = ParsedFunction
  expression = 'x + sin(y)'
[]

[UserObjects/study]
  type = RepeatableRayStudy
  names = 'diag
           top_across
           bottom_across
           partial'
  start_points = '0 0 0
                  0 5 0
                  0 0 0
                  0.5 0.5 0'
  end_points = '5 5 0
                5 5 0
                5 0 0
                4.5 0.5 0'
[]

[RayKernels/function_integral]
  type = FunctionIntegralRayKernel
  function = parsed_function
  rays = 'diag top_across bottom_across partial'
[]

[Postprocessors]
  [diag_value]
    type = RayIntegralValue
    ray_kernel = function_integral
    ray = diag
  []
  [top_across_value]
    type = RayIntegralValue
    ray_kernel = function_integral
    ray = top_across
  []
  [bottom_across_value]
    type = RayIntegralValue
    ray_kernel = function_integral
    ray = bottom_across
  []
  [partial_value]
    type = RayIntegralValue
    ray_kernel = function_integral
    ray = partial
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = false
  csv = true
[]
