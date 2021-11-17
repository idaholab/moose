[Mesh]
[]

[AuxVariables]
  [temperature_forward]
  []
  [T2]
  []
[]
[AuxKernels]
  [TT]
    type = ParsedAux
    args = 'temperature temperature_forward'
    variable = T2
    function = 'temperature*(100-temperature_forward)'
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
  [pt]
    type = ReporterPointSource
    variable = temperature
    x_coord_name = point_source/x
    y_coord_name = point_source/y
    z_coord_name = point_source/z
    value_name = point_source/value
  []
[]


[BCs]
  [left]
    type = ConvectiveFluxFunction
    variable = temperature
    boundary = 'left'
    T_infinity = 0.0
    coefficient = function1
  []
  [right]
    type = NeumannBC
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

[Functions]
  [function1]
    type = ParsedFunction
    value = a*1.0
    vars = 'a'
    vals = 'p1'
  []
[]

[Postprocessors]
  [adjoint_pt_0]
    type = SideIntegralVariablePostprocessor
    variable = T2
    boundary = left
  []
  [p1]
    type = ConstantValuePostprocessor
    value = 1
    execute_on = LINEAR
  []
[]

[Reporters]
  [point_source]
    type = ConstantReporter
    real_vector_names = 'x y z value'
    real_vector_values = '0.1	0.1	0.1	0.1	0.1	0.1	0.1	0.1	0.1	0.1	0.1	0.1	0.1	0.1	0.1	0.1	0.1	0.1	0.1	0.1	0.1;
             0	0.1	0.2	0.3	0.4	0.5	0.6	0.7	0.8	0.9	1	1.1	1.2	1.3	1.4	1.5	1.6	1.7	1.8	1.9	2;
             0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0;
            10	10	10	10	10	10	10	10	10	10	10	10	10	10	10	10	10	10	10	10	10'
  []
[]

[VectorPostprocessors]
  [adjoint_pt]
    type = VectorOfPostprocessors
    postprocessors = 'adjoint_pt_0'
  []
[]

[Controls]
  [adjointReceiver]
    type = ControlsReceiver
  []
[]

[Outputs]
  exodus = true
  file_base = 'adjoint'
[]
