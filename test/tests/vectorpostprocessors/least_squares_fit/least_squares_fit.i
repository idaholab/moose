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
  [../]
  [./least_squares_fit_sample]
    type = LeastSquaresFit
    vectorpostprocessor = line_sample
    x_name = 'id'
    y_name = 'u'
    order = 1
    num_samples = 20
    output = samples
  [../]
  [./least_squares_fit_coeffs]
    type = LeastSquaresFit
    vectorpostprocessor = line_sample
    x_name = 'id'
    y_name = 'u'
    order = 1
    output = coefficients
  [../]
  [./shift_and_scale_x_least_squares_fit_sample]
    type = LeastSquaresFit
    vectorpostprocessor = line_sample
    x_name = 'id'
    y_name = 'u'
    x_shift = 1
    x_scale = 10
    order = 1
    num_samples = 20
    output = samples
  [../]
  [./shift_and_scale_x_least_squares_fit_coeffs]
    type = LeastSquaresFit
    vectorpostprocessor = line_sample
    x_name = 'id'
    y_name = 'u'
    x_shift = 1
    x_scale = 10
    order = 1
    output = coefficients
  [../]
  [./shift_and_scale_y_least_squares_fit_sample]
    type = LeastSquaresFit
    vectorpostprocessor = line_sample
    x_name = 'id'
    y_name = 'u'
    y_shift = 1
    y_scale = 10
    order = 1
    num_samples = 20
    output = samples
  [../]
  [./shift_and_scale_y_least_squares_fit_coeffs]
    type = LeastSquaresFit
    vectorpostprocessor = line_sample
    x_name = 'id'
    y_name = 'u'
    y_shift = 1
    y_scale = 10
    order = 1
    output = coefficients
  [../]
[]

[Executioner]
  # Preconditioned JFNK (default)
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  file_base = out
  execute_on = 'timestep_end'
  csv = true
[]
