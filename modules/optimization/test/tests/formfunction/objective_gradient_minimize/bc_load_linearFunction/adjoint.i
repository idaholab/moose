[Mesh]
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temperature
  []
[]

[DiracKernels]
  [pt]
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
    type = NeumannBC
    variable = temperature
    boundary = left
    value = 0
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
    type = GenericConstantMaterial
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
  [left_constant_deriv_a]
    type = ParsedFunction
    value = 1.0
  []
  [left_linear_deriv_b]
    type = ParsedFunction
    value = y
  []
[]

[Postprocessors]
  [adjoint_bc_0]
    type = VariableFunctionSideIntegral
    boundary = left
    function = left_constant_deriv_a
    variable = temperature
  []
  [adjoint_bc_1]
    type = VariableFunctionSideIntegral
    boundary = left
    function = left_linear_deriv_b
    variable = temperature
  []
[]

[VectorPostprocessors]
  [point_source]
    type = ConstantVectorPostprocessor
    vector_names = 'x y z value'
    value = '0.2 0.8 0.2 0.8;
             0.2 0.6 1.4 1.8;
             0   0   0   0;
             10  10  10  10'
  []
  [adjoint_bc]
    type = VectorOfPostprocessors
    postprocessors = 'adjoint_bc_0 adjoint_bc_1'
  []
[]

[Outputs]
  console = true
  exodus = true
  file_base = 'adjoint'
[]
