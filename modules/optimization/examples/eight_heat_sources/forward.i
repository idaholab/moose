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
  [ar]
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

[Problem]#do we need this
  type = FEProblem
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
    value = '0.3 0.3 0.3 0.3 0.7 0.7 0.7 0.7;
             0.8 0.6 0.4 0.2 0.8 0.6 0.4 0.2;
             0   0   0   0   0   0   0   0;
             0   0   0   0   0   0   0   0'
  []
  [dr]
    type = MeasuredDataPointSampler
    variable = temperature
    points = '0.5 0.1 0
              0.5 0.2 0
              0.5 0.3 0
              0.5 0.4 0
              0.5 0.5 0
              0.5 0.6 0
              0.5 0.7 0
              0.5 0.8 0
              0.5 0.9 0

              0.9 0.1 0
              0.9 0.2 0
              0.9 0.3 0
              0.9 0.4 0
              0.9 0.5 0
              0.9 0.6 0
              0.9 0.7 0
              0.9 0.8 0
              0.9 0.9 0

              0.1 0.1 0
              0.1 0.2 0
              0.1 0.3 0
              0.1 0.4 0
              0.1 0.5 0
              0.1 0.6 0
              0.1 0.7 0
              0.1 0.8 0
              0.1 0.9 0'
    measured_values = '6.65 12.04 15.13 16.17 15.50 13.65 10.89 7.57 3.87
                       1.88 3.41  4.26  4.55  4.36  3.84  3.07  2.14 1.09
                       3.00 5.38  6.61  6.93  6.52  5.66  4.47  3.09 1.57'
  []
[]

[Outputs]
  console = false
  exodus = true
  file_base = 'forward'
[]
