[Mesh]
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[Kernels]
  [TensorMechanics]
    use_displaced_mesh = false
    displacements ='disp_x disp_y'
  []
[]

#-----adjoint problem information------------------
[DiracKernels]
  [pt]
    type = ReporterPointSource
    variable = disp_y
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
  [params]
    type = ConstantReporter
    real_vector_names = 'right_fy_value'
    real_vector_values = '0' # Dummy value
  []
[]

[Functions]
  [right_fy_func]
    type = ParsedOptimizationFunction
    expression = 'val'
    param_symbol_names = 'val'
    param_vector_name = 'params/right_fy_value'
  []
[]

[VectorPostprocessors]
  [adjoint_pt]
    type = SideOptimizationNeumannFunctionInnerProduct
    variable = disp_y
    function = right_fy_func
    boundary = right
  []
[]

#---------------------------------------------------

[BCs]
  [left_ux]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [left_uy]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0
  []
  [right_fy]
    type = NeumannBC
    variable = disp_y
    boundary = right
    value = 0 #2000
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 10e3
    poissons_ratio = 0.3
  []
  [strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y'
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = false
  console = false
  exodus = false
  file_base = 'adjoint'
[]

