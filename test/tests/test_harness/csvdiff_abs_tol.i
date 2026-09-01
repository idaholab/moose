# Tests when CSVDiff should pass a diff.
#
#                        val1      val2
#                 ----------------------
# gold values    |       1e-6 |    1e-6 |
# test values    | 1.00001e-6 | 1.01e-6 |
# absolute error |      1e-11 |    1e-8 |
# relative error |      ~1e-5 | ~0.0099 |

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [val1]
    type = ConstantPostprocessor
    value = 1.00001e-6
  []
  [val2]
    type = ConstantPostprocessor
    value = 1.01e-6
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL'
[]
