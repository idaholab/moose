[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = halfSphere.e
  []
  [patch]
    type = PatchSidesetGenerator
    boundary = 2
    n_patches = 10
    input = fmg
  []
[]

[Variables]
  [temperature]
  []
[]

[AuxVariables]
  [saved_t]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
    save_in = saved_t
  []
[]

[DiracKernels]
    [pt]
      type = ReporterPointSource
      variable = temperature
      x_coord_name = 'point_source/x'
      y_coord_name = 'point_source/y'
      z_coord_name = 'point_source/z'
      value_name = 'point_source/value'
    []
[]

[BCs]
  [round]
    type = ConvectiveFluxFunction
    boundary = round
    variable = temperature
    coefficient = 0.05
    T_infinity = 100.0
  []
  [flat]
    type = NeumannBC
    variable = temperature
    boundary = flat
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
    type = ConstantVectorPostprocessor
    vector_names = 'x y z value'
    value = '4 2.3  2.3  2.3  2.3;
             0   2.3 -2.3 -2.3  2.3;
             0   2.3  2.3 -2.3 -2.3;
             100 150  300  250  150'
    execute_on = LINEAR
  []
  #-----every forward problem should have these two
  [data_pt]
    type = VppPointValueSampler
    variable = temperature
    reporter_name = measure_data
  []
[]

[Reporters]
  [measure_data]
    type=OptimizationData
  []
[]

[Outputs]
  console = true #false
  exodus = true
  csv = true
  file_base = 'forward'
[]
