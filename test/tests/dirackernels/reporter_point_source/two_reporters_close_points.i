[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -10.22e-3
    xmax = 0
    ymin = 0
    ymax = 10.16e-3
    nx = 11
    ny = 11
  []
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
[]

[DiracKernels]
  [measurement_point_source]
    type = ReporterPointSource
    variable = u
    value_name = 'measurement_data/value'
    x_coord_name = 'measurement_data/x'
    y_coord_name = 'measurement_data/y'
    z_coord_name = 'measurement_data/z'
  []
  [reference_point_source]
    type = ReporterPointSource
    variable = u
    value_name = 'reference_data/value'
    x_coord_name = 'reference_data/x'
    y_coord_name = 'reference_data/y'
    z_coord_name = 'reference_data/z'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
[]

[Reporters]
  [measurement_data]
    type = ConstantReporter
    real_vector_names = 'x y z value'
    real_vector_values = '-0.002404676911788595; 0.005612893359425962; 0.0; 0.0'
  []
  [reference_data]
    type = ConstantReporter
    real_vector_names = 'x y z value'
    real_vector_values = '-2.404677e-03; 5.612893e-03; 0; 1'
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
[]
