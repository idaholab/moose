[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
  []
[]

[Problem]
  kernel_coverage_check = false
  skip_nl_system_check = true
  solve = false
[]

[UserObjects]
  [solution]
    type = SolutionUserObject
    system = aux0
    mesh = point_value_wrapper_xyz_source_0001_mesh.xda
    es = point_value_wrapper_xyz_source_0001.xda
    system_variables = source_xyz
  []
[]

[Postprocessors]
  [xyz_average]
    type = TestSolutionPointValueWrapper
    variable = source_xyz
    point = '0.5 0.5 0'
    weighting_type = average
    solution = solution
    execute_on = INITIAL
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = INITIAL
[]
