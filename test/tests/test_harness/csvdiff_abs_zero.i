# Tests when CSVDiff compares a value below the absolute zero (floor) to a value above it.
#
# The gold value is below the default abs_zero of 1e-10 but the test value is not, so the
# comparison is not skipped (it is only skipped when both values are below abs_zero).
#
#                              val
#                 ----------------
# gold value      |          9e-11 |
# test value      |          3e-10 |
# absolute error  |        2.1e-10 |
# relative error  |            0.7 |

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
