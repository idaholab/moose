[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 2
  xmin = 0.0
  xmax = 5.0
  ymin = 0.0
  ymax = 1.0
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxVariables]
  [stress00]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress11]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress12]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [stress00]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
    variable = stress00
  []
  [stress11]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 1
    index_j = 1
    variable = stress11
  []
  [stress12]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 1
    index_j = 2
    variable = stress12
  []
[]

[Modules/TensorMechanics/Master]
  displacements = 'disp_x disp_y'
  [strain_calculator]
    strain = SMALL
    displacements = 'disp_x disp_y'
    add_variables = true
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
    value = 0 #2000
  []
[]

[Materials]
  [./elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 10e9
    poissons_ratio = 0.3
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
  [../]
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
