[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 4
  nz = 2
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 2.0
  zmin = 0.0
  zmax = 1.0
  elem_type = HEX8
[]

[MeshModifiers]
  [./rotate]
    type = Transform
    transform = ROTATE
    vector_value = '0 -20 -60'
  [../]
[]

[Modules/TensorMechanics/Master/All]
  strain = FINITE
  add_variables = true
[]

[BCs]
  [./Pressure]
    [./top]
      boundary = top
      function = '-1000*t'
    [../]
  [../]
  [./right_x]
    type = PenaltyInclinedBC
    variable = disp_x
    boundary = right
    penalty = 1.0e8
    component = 0
  [../]
  [./right_y]
    type = PenaltyInclinedBC
    variable = disp_y
    boundary = right
    penalty = 1.0e8
    component = 1
  [../]
  [./right_z]
    type = PenaltyInclinedBC
    variable = disp_z
    boundary = right
    penalty = 1.0e8
    component = 2
  [../]
  [./bottom_x]
    type = PenaltyInclinedBC
    variable = disp_x
    boundary = bottom
    penalty = 1.0e8
    component = 0
  [../]
  [./bottom_y]
    type = PenaltyInclinedBC
    variable = disp_y
    boundary = bottom
    penalty = 1.0e8
    component = 1
  [../]
  [./bottom_z]
    type = PenaltyInclinedBC
    variable = disp_z
    boundary = bottom
    penalty = 1.0e8
    component = 2
  [../]
  [./back_x]
    type = PenaltyInclinedBC
    variable = disp_x
    boundary = back
    penalty = 1.0e8
    component = 0
  [../]
  [./back_y]
    type = PenaltyInclinedBC
    variable = disp_y
    boundary = back
    penalty = 1.0e8
    component = 1
  [../]
  [./back_z]
    type = PenaltyInclinedBC
    variable = disp_z
    boundary = back
    penalty = 1.0e8
    component = 2
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu     superlu_dist'

  # controls for linear iterations
  l_max_its = 10
  l_tol = 1e-4

  # controls for nonlinear iterations
  nl_max_its = 100
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12

  # time control
  start_time = 0.0
  dt = 1
  end_time = 5
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Outputs]
  exodus = true
[]
