###############################################################
# The following tests that the CSV output object can include an
# additional .csv file that contains the time and timestep
# data from VectorPostprocessor object.
###############################################################


[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
  [./v]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./diff_v]
    type = Diffusion
    variable = v
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
  [./left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 1
  [../]
  [./right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 0
  [../]
[]

[VectorPostprocessors]
  [./line_sample]
    type = LineValueSampler
    variable = 'u v'
    start_point = '0 0.5 0'
    end_point = '1 0.5 0'
    num_points = 11
    sort_by = id
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 20
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  execute_on = 'initial timestep_end'
  file_base = 'time_data'
  [./out]
    type = CSV
    time_data = true
    interval = 2
  [../]
[]
