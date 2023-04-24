[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward.i
    execute_on = INITIAL
  []
[]

[AuxVariables/u_reverse]
[]

[UserObjects]
  [u_reverse_solution]
    type = AdjointSolutionUserObject
    mesh = forward_out.e
    system_variables = 'u'
    reverse_time_end = 10
  []
  [terminate]
    type = Terminator
    expression = 'u_reverse_test > 1e-12'
    error_level = ERROR
  []
[]

[AuxKernels]
  [u_reverse_aux]
    type = SolutionAux
    variable = u_reverse
    solution = u_reverse_solution
  []
[]

[Functions]
  [u_reverse_fun]
    type = ParsedFunction
    expression = '(x + y) * (11 - t)'
  []
[]

[Postprocessors]
  [u_reverse_test]
    type = ElementL2Error
    variable = u_reverse
    function = u_reverse_fun
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 10
[]
