[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 100
    ny = 100
    xmax = 1
    ymax = 1
  []
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
  [a]
    type = VectorPostprocessorPointSource
    variable = temperature
    vector_postprocessor = point_source
    x_coord_name = x
    y_coord_name = y
    z_coord_name = z
    value_name = value
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
    type = ParConstantVectorPostprocessor
    vector_names = 'x y z value'
    value = '0.5 0.5 0.5 0.5 0.5 0.5 0.5 0.5 0.5
             0.9 0.9 0.9 0.9 0.9 0.9 0.9 0.9 0.9
             0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1;

             0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9
             0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9
             0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9;

             0   0   0   0   0   0   0   0   0
             0   0   0   0   0   0   0   0   0
             0   0   0   0   0   0   0   0   0;

             7.5 7.5 7.5 7.5 7.5 7.5 7.5 7.5 7.5
             7.5 7.5 7.5 7.5 7.5 7.5 7.5 7.5 7.5
             7.5 7.5 7.5 7.5 7.5 7.5 7.5 7.5 7.5'
  []
  [ar]
    type = PointValueSampler
    variable = temperature
    points = '0.3 0.8 0
              0.3 0.6 0
              0.3 0.4 0
              0.3 0.2 0
              0.7 0.8 0
              0.7 0.6 0
              0.7 0.4 0
              0.7 0.2 0'
    sort_by = id
  []
[]

[Outputs]
  console = false
  exodus = true
  file_base = 'adjoint'
[]
