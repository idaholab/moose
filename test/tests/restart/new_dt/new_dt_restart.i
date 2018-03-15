[Mesh]
  file = new_dt_out_cp/0010_mesh.cpr
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10

  # Here we are supplying a different dt
  dt = 0.25
  start_time = 1.0

  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  [./exodus]
    type = Exodus
    execute_on = 'timestep_end final'
  [../]
[]

[Problem]
  restart_file_base = new_dt_out_cp/0010
[]
