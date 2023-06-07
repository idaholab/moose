[Mesh]
  # This test uses SolutionUserObject which doesn't work with DistributedMesh.
  type = GeneratedMesh
  dim = 1
  nx = 1
  parallel_type = replicated
[]

[Variables]
  [u]
    family = SCALAR
    order = FIRST
  []
[]

[AuxVariables]
  [a]
    family = SCALAR
    order = FIRST
  []
[]

[ICs]
  [u]
    type = ScalarSolutionIC
    variable = u
    solution_uo = solution_uo
    from_variable = a
  []
  [a]
    type = ScalarSolutionIC
    variable = a
    solution_uo = solution_uo
    from_variable = a
  []
[]

[UserObjects]
  [solution_uo]
    type = SolutionUserObject
    # Generated from ../../auxkernels/solution_scalar_aux/build.i
    mesh = 'build_out.e'
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL'
[]
