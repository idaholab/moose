[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -10e-3
    xmax = 0
    ymin = 0
    ymax = 10e-3
    nx = 10
    ny = 10
  []
[]

[Problem]
  extra_tag_vectors = 'measurement reference'
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [measurement_residual]
    order = FIRST
    family = LAGRANGE
  []
  [reference_residual]
    order = FIRST
    family = LAGRANGE
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
    extra_vector_tags = 'measurement'
  []
  [reference_point_source]
    type = ReporterPointSource
    variable = u
    value_name = 'reference_data/value'
    x_coord_name = 'reference_data/x'
    y_coord_name = 'reference_data/y'
    z_coord_name = 'reference_data/z'
    extra_vector_tags = 'reference'
  []
[]

[AuxKernels]
  [measurement_residual_aux]
    type = TagVectorAux
    variable = measurement_residual
    v = u
    vector_tag = measurement
  []
  [reference_residual_aux]
    type = TagVectorAux
    variable = reference_residual
    v = u
    vector_tag = reference
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
    real_vector_values = '-5.0000001e-3; 5e-3; 0.0; 2'
    outputs = none
  []
  [reference_data]
    type = ConstantReporter
    real_vector_names = 'x y z value'
    real_vector_values = '-5e-3; 5e-3; 0.0; 3'
    outputs = none
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Postprocessors]
  [measurement_residual_at_point]
    type = PointValue
    variable = measurement_residual
    point = '-5e-3 5e-3 0.0'
  []
  [reference_residual_at_point]
    type = PointValue
    variable = reference_residual
    point = '-5e-3 5e-3 0.0'
  []
[]

[Outputs]
  csv = true
[]
