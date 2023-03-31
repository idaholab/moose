[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
[]

[Problem]
  previous_nl_solution_required = true
[]

[Variables]
  [u]
    type = MooseVariableFVReal
    initial_condition = 1
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = u
    coeff = 1
  []
  [rxn]
    type = FVSecondOrderRxnLagged
    variable = u
    lag = false
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = FVDirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  line_search = 'none'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
