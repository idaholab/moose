[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [dt]
    type = ADTimeDerivative
    variable = u
  []
  [diff]
    type = ADDiffusion
    variable = u
  []
  [source]
    type = ADBodyForce
    variable = u
    value = 15
  []
[]

[BCs]
  [top]
    type = DirichletBC
    variable = u
    value = 1
    boundary = 'left'
  []
  [bc_all]
    type = ADPenaltyLimitBC
    variable = u
    value = 7
    boundary = 'right'
    penalty = 10000
    apply_penalty_when = greaterthan
  []
[]


[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  line_search = none
  nl_abs_tol = 1e-10
  num_steps = 10
  dt = 1
[]

[Outputs]
  exodus = true
  csv = true
[]
