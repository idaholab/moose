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
    type = ReporterPointSource
    variable = temperature
    x_coord_name = misfit/measurement_xcoord
    y_coord_name = misfit/measurement_ycoord
    z_coord_name = misfit/measurement_zcoord
    value_name = misfit/misfit_values
  []
[]

[Reporters]
  [misfit]
    type=OptimizationData
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
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  petsc_options_iname = '-ksp_type -pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'preonly lu       superlu_dist'
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
  [adjoint_bc]
    type = VectorOfPostprocessors
    postprocessors = 'adjoint_bc_0 adjoint_bc_1'
  []
[]

[Outputs]
  console = true
  #exodus = true
  file_base = 'adjoint'
[]
