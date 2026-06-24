[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 5
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[Kernels]
  [diff_u]
    type = ADDiffusion
    variable = u
  []
  [diff_v]
    type = ADDiffusion
    variable = v
  []
[]

[BCs]
  inactive = 'left_penalty_u left_penalty_v right_penalty_u right_penalty_v'
  [left_penalty_u]
    type = ADPenaltyDirichletBC
    variable = u
    boundary = left
    value = 0
    penalty = 1e6
  []
  [right_penalty_u]
    type = ADPenaltyDirichletBC
    variable = u
    boundary = right
    value = 1
    penalty = 1e6
  []
  [left_penalty_v]
    type = ADPenaltyDirichletBC
    variable = v
    boundary = left
    value = 0
    penalty = 1e6
  []
  [right_penalty_v]
    type = ADPenaltyDirichletBC
    variable = v
    boundary = right
    value = 1
    penalty = 1e6
  []
  [left_u]
    type = ADDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right_u]
    type = ADDirichletBC
    variable = u
    boundary = right
    value = 1
  []
  [left_v]
    type = ADDirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [right_v]
    type = ADDirichletBC
    variable = v
    boundary = right
    value = 1
  []
[]

[Convergence]
  [nl0]
    type = DefaultNonlinearConvergence
    nl_abs_tol = 1e-10
    nl_rel_tol = 0
  []
  [nl1]
    type = DefaultNonlinearConvergence
    nl_abs_tol = 1e-10
    nl_rel_tol = 0
  []
[]

[Postprocessors]
  [u]
    type = ElementAverageValue
    variable = u
    execute_on = 'final'
  []
  [v]
    type = ElementAverageValue
    variable = v
    execute_on = 'final'
  []
[]

[Outputs]
  console = false
[]
