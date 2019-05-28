[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables/v]
  initial_condition = 111
[]

[Kernels]
  [diff]
    type = ADDiffusion
    variable = v
  []
  [time]
    type = ADTimeDerivative
    variable = v
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = v
    boundary = left
    value = 111
  []
  [right]
    type = DirichletBC
    variable = v
    boundary = right
    value = 112
  []
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 1
  solve_type = 'NEWTON'
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = FINAL
    file_base = sub_final
  []
[]
