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
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = FunctionDirichletBC
    variable = u
    boundary = right
    function = 't'
  [../]
  [./left_v]
    type = FunctionDirichletBC
    variable = v
    boundary = left
    function = 't'
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
    outputs = none
  [../]
  [./least_squares_fit_coeffs]
    type = LeastSquaresFitHistory
    vectorpostprocessor = line_sample
    x_name = 'id'
    y_name = 'u'
    order = 1
  [../]
  [./shift_and_scale_x_least_squares_fit_coeffs]
    type = LeastSquaresFitHistory
    vectorpostprocessor = line_sample
    x_name = 'id'
    y_name = 'u'
    x_shift = 1
    x_scale = 10
    order = 1
  [../]
  [./shift_and_scale_y_least_squares_fit_coeffs]
    type = LeastSquaresFitHistory
    vectorpostprocessor = line_sample
    x_name = 'id'
    y_name = 'u'
    y_shift = 1
    y_scale = 10
    order = 1
  [../]
[]

[Executioner]
  type = Transient
  start_time = 0.0
  num_steps = 3
  dt = 1.0
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  file_base = out
  execute_on = 'timestep_end'
  csv = true
[]
