# Tests the LinearCombinationPostprocessor post-processor, which computes
# a linear combination of an arbitrary number of post-processor values.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [./pp1]
    # number of elements, equal to 2
    type = NumElements
  [../]
  [./pp2]
    # number of nodes, equal to 3
    type = NumNodes
  [../]

  # post-processor value being tested; value should be the following:
  #   value = c1 * pp1 + c2 * pp2 + b
  #         = 2  * 2   + -1 * 3   + 5 = 6
  [./linear_combination]
    type = LinearCombinationPostprocessor
    pp_names = 'pp1 pp2'
    pp_coefs = '2   -1'
    b = 5
  [../]
[]

[Outputs]
  show = linear_combination
  csv = true
[]
