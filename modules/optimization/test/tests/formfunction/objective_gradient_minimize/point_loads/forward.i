[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 20
  xmax = 1
  ymax = 2
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
  []
[]

[DiracKernels]
  [pt]
    type = VectorPostprocessorPointSource
    variable = temperature
    vector_postprocessor = point_source
    x_coord_name = 'x'
    y_coord_name = 'y'
    z_coord_name = 'z'
    value_name = 'value'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = temperature
    boundary = right
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = temperature
    boundary = bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = temperature
    boundary = top
    value = 0
  []
[]

[Materials]
  [steel]
    type = ADGenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 5
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[VectorPostprocessors]
  [point_source]
    type = ConstantVectorPostprocessor
    vector_names = 'x y z value'
    value = '0.2 0.2 0.8; 0.2 0.8 0.2; 0 0 0; -2458 7257 26335'
  []
  [data_pt]
    type = MeasuredDataPointSampler
    variable = temperature
    points = '0.3 0.3 0
              0.4 1.0 0
              0.8 0.5 0
              0.8 0.6 0'
    measured_values = '100 204 320 216'
  []
[]

[Outputs]
  console = true
  exodus = true
  file_base = 'forward'
[]
