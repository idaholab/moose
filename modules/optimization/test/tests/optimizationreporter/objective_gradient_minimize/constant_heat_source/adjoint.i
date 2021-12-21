[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 2
  ymax = 2
[]

[Variables]
  [adjoint_T]
  []
[]

[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    variable = adjoint_T
  []
[]

[DiracKernels]
  [pt]
    type = ReporterPointSource
    variable = adjoint_T
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
    variable = adjoint_T
    boundary = left
    value = 0
  []
  [right]
    type = NeumannBC
    variable = adjoint_T
    boundary = right
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = adjoint_T
    boundary = bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = adjoint_T
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
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  petsc_options_iname = '-ksp_type -pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'preonly lu       superlu_dist'
[]

[Functions]
  [volumetric_heat_func_deriv]
    type = ParsedFunction
    value = dq
    vars = 'dq'
    vals = 1
  []
[]

[Postprocessors]
  # integral of load function gradient w.r.t parameter
  [heatSourceGradient]
    type = VariableFunctionElementIntegral
    function = volumetric_heat_func_deriv
    variable = adjoint_T
  []
[]

[VectorPostprocessors]
  [gradient_vpp]
    type = VectorOfPostprocessors
    postprocessors = 'heatSourceGradient'
  []
[]


[Outputs]
  # console = true
  # exodus = true
  file_base = 'adjoint'
[]
