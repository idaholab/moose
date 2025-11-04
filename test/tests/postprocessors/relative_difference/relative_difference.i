# Tests the RelativeDifferencePostprocessor post-processor, which computes
# the relative difference between 2 post-processor values.

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
  [./num_elems]
    # number of elements, equal to 2
    type = NumElements
  [../]
  [./num_nodes]
    # number of nodes, equal to 3
    type = NumNodes
  [../]
  [./zero]
    # zero post-processor value
    type = EmptyPostprocessor
  [../]

  # For the case in this input file, this will be computed as
  #   y = abs((num_nodes - num_elems) / num_elems)
  #   y = abs((3         - 2        ) / 2        ) = 0.5
  # When the command-line modification "Postprocessors/relative_difference/value2=zero" is used,
  #   y = abs(num_nodes - zero)
  #   y = abs(3         - 0   ) = 3
  [./relative_difference]
    type = RelativeDifferencePostprocessor
    value1 = num_nodes
    value2 = num_elems
  [../]
[]

[Outputs]
  [./out]
    type = CSV
    show = relative_difference
  [../]
[]
