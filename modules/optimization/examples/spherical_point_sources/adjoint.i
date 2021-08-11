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
  [round]
    type = ConvectiveFluxFunction
    boundary = 2
    variable = temperature
    coefficient = 0.1
    T_infinity = 0.0
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
    value = '4.24	4.24	4.24	 4.24	2.45	2.45	 2.45	 2.45 4.9;
                0	2.45	   0	-2.45	   0	4.24	    0	-4.24   0;
             2.45	   0 -2.45	    0	4.24	0	    -4.24	    0   0;

             1 2 3 4 5 6 7 8 9'
  []
  [adjoint_meas]
    type = PointValueSampler
    variable = temperature
    points = '4 0 0
              2 2 2
              2 -2 2
              2 -2 -2
              2 2 -2'
    sort_by = id
  []
[]

[Outputs]
  console = true
  exodus = true
  file_base = 'adjoint'
[]
