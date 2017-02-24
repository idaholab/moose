# This tests for the bug https://github.com/idaholab/moose/issues/8575.
# It makes sure that the material property dependency checking accounts for
# the fact that materials provided on "ANY_BLOCK" count as satisfying requests
# for that property on all block IDs.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[GlobalParams]
  block = 0
[]

[AddMatAndKernel]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
[]

