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
    value = 10
    boundary = 'right'
    penalty = 10000
    apply_penalty_when = lessthan
  []
[]


[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  nl_abs_tol = 1e-10
  num_steps = 3
  dt = 1
[]

[Outputs]
  exodus = true
  csv = true
[]
