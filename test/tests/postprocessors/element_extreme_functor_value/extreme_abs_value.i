[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    xmax = 4
    nx = 36
  []
[]

[Functions]
  [f1]
    type = PiecewiseLinear
    x = '0   0.5 1.5  2.5  3.5 4'
    y = '0.0 0.5 0.0 -1.5 -0.5 0.0'
    axis = X
  []
  [f2]
    type = PiecewiseLinear
    x = '0   0.5 1.5  2.5  3.5 4'
    y = '1.0 0.0 -1.0 0.0  0.5 1.0'
    axis = X
  []
[]

[Postprocessors]
  [f1_max]
    type = ElementExtremeFunctorValue
    functor = f1
    value_type = MAX_ABS
    execute_on = 'initial timestep_end'
  []
  [f2_proxy_max]
    type = ElementExtremeFunctorValue
    functor = f2
    proxy_functor = f1
    value_type = MAX_ABS
    execute_on = 'initial timestep_end'
  []
  [test]
    type = ParsedPostprocessor
    expression = 'abs(f1_max + 1.5) + abs(f2_proxy_max)'
    pp_names = 'f1_max f2_proxy_max'
    execute_on = 'initial timestep_end'
  []
[]

[UserObjects]
  [kill]
    type = Terminator
    expression = 'test > 0'
    error_level = ERROR
    fail_mode = HARD
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
