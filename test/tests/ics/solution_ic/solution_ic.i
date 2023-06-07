[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  # This test uses SolutionUserObject which doesn't work with DistributedMesh.
  parallel_type = replicated
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
  [u_elem]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxVariables]
  [u_aux]
    order = FIRST
    family = LAGRANGE
  []
  [u_aux_elem]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[ICs]
  [initial_cond_nl]
    type = SolutionIC
    solution_uo = exo_soln
    variable = u
    from_variable = 'u'
    block = 0
  []
  [initial_cond_nl_elem]
    type = SolutionIC
    solution_uo = exo_soln
    variable = u_elem
    from_variable = 'u_elem'
  []
  [initial_cond_aux]
    type = SolutionIC
    solution_uo = exo_soln
    variable = u_aux
    from_variable = 'u_aux'
  []
  [initial_cond_aux_elem]
    type = SolutionIC
    solution_uo = exo_soln
    variable = u_aux_elem
    from_variable = 'u_aux_elem'
  []
[]

[UserObjects]
  [exo_soln]
    type = SolutionUserObject
    mesh = 'gold/solution_ic_out.e'
    system_variables = 'u u_elem u_aux u_aux_elem'
    timestep = LATEST
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
  execute_on = 'INITIAL'
[]
