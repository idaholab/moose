[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  nl_sys_names = 'uv_sys w_sys'
  solve = false
  kernel_coverage_check = false
[]

# u and w are both variable number 0 in their own system, and a is number 0 in the auxiliary
# system, so their numbers alone cannot tell them apart
[Variables]
  [u]
    solver_sys = uv_sys
  []
  [v]
    solver_sys = uv_sys
  []
  [w]
    solver_sys = w_sys
  []
[]

[AuxVariables]
  [a]
  []
[]

[Kernels]
  [test]
    type = JvarMapInterfaceTestKernel
    variable = v
    # u is listed first so that the variables of other systems are coupled after it and would
    # overwrite its map entry if they were not skipped
    coupled_variables = 'u w a'
    v0 = 'u w a'
  []
[]

[Executioner]
  type = Steady
[]
