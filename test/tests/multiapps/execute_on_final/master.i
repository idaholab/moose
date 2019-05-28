[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables/u]
[]

[Kernels]
  [diff]
    type = ADDiffusion
    variable = u
  []
  [time]
    type = ADTimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 4
  dt = 0.25
  solve_type = 'NEWTON'
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = FINAL
    file_base = master_final
  []
[]

[MultiApps]
  [sub_app]
    positions = '0 0 0'
    type = TransientMultiApp
    input_files = 'sub.i'
    app_type = MooseTestApp
  []
[]
