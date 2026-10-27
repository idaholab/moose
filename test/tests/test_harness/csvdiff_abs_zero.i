# Tests when CSVDiff compares a value below the absolute zero (floor) to a value above it.
#
# The gold value is below the default abs_zero of 1e-10, so it is compared as 0.
#
#                              val
#                 ----------------
# gold value      |          9e-11 |
# test value      |          3e-10 |
# comparison      |   0 vs. 3e-10 |
# absolute error  |          3e-10 |
# relative error  |              1 |

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [val]
    type = ConstantPostprocessor
    value = 3e-10
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL'
[]
