[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  parallel_type = replicated
[]

[UserObjects]
  [solution]
    type = SolutionUserObject
    mesh = point_value_cache_source_out.e
    system_variables = source_value
    timestep = LATEST
  []
[]

[Postprocessors]
  [cache_error]
    type = TestSolutionPointValueCache
    solution = solution
    variable = source_value
    prime_point = '0.25 0.5 0'
    test_point = '0.75 0.5 0'
    source_subdomain = 1
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
