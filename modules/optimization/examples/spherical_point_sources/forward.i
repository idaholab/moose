[Mesh]
 [./fmg]
   type = FileMeshGenerator
   file = halfSphere.e
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
  [round]
    type = ConvectiveFluxFunction
    boundary = 2
    variable = temperature
    coefficient = 0.05
    T_infinity = 100.0
  []
  [flat]
    type = NeumannBC
    variable = temperature
    boundary = 1
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
    value = '4 2.3  2.3  2.3  2.3;
             0   2.3 -2.3 -2.3  2.3;
             0   2.3  2.3 -2.3 -2.3;
             100 150  300  250  150'
  []
  [forward_meas]
    type = MeasuredDataPointSampler
    variable = temperature
    points =
    '4.24	0	2.45
     4.24	2.45	0
     4.24	0	-2.45
     4.24	-2.45	0
     2.45	0	4.24
     2.45	4.24	0
     2.45	0	-4.24
     2.45	-4.24	0
     4.9 0 0'
    measured_values = '221.5542848	220.3392602	221.1322675	222.3562443	220.9615351	219.2140933	220.3868245	222.1432008	222.9280329'
  []
[]

[Outputs]
  console = true #false
  exodus = true
  csv = true
  file_base = 'forward'
[]
