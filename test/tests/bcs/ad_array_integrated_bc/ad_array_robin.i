# Robin boundary condition grad(u).n = h - alpha u on a two-component array variable. The same
# problem written with two standard variables is solved in robin_reference.i; both write the
# component integrals to the same gold file.
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 6
  ny = 6
[]

[Variables]
  [u]
    components = 2
  []
[]

[Kernels]
  [diff]
    type = ADArrayDiffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = ADArrayDirichletBC
    variable = u
    boundary = left
    values = '1 2'
  []
  [right]
    type = ADArrayRobinBC
    variable = u
    boundary = right
    value = '0.5 -1'
    alpha = '2 0'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_rel_tol = 1e-12
[]

[Postprocessors]
  [int0]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    component = 0
  []
  [int1]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    component = 1
  []
[]

[Outputs]
  file_base = robin_out
  csv = true
[]
