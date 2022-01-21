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
    value = 2000
  []
[]

[Materials]
  [./elasticity]
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

[VectorPostprocessors]
  [point_sample]
    type = PointValueSampler
    variable = 'disp_y'
    points = '5.0 1.0 0'
    sort_by = x
  []
  [data_pt]
    type = VppPointValueSampler
    variable = disp_y
    reporter_name = measure_data
  []
[]

[Reporters]
  [measure_data]
    type=OptimizationData
  []
[]

[Controls]
  [parameterReceiver]
    type = ControlsReceiver
  []
[]

[Outputs]
  csv = false
  console = false
  exodus = false
  file_base = 'forward'
[]

