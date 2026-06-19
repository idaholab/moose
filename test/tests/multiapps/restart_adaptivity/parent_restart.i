[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [time]
    type = TimeDerivative
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

[Problem]
  restart_file_base = parent_out_cp/0001
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 0.1
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    input_files = sub.i
    execute_on = timestep_end
  []
[]
