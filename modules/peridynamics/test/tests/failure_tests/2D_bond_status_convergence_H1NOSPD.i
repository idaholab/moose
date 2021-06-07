
[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = PeridynamicsMesh
  horizon_number = 3
  cracks_start = '0.25 0.5 0'
  cracks_end = '0.75 0.5 0'

  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 8
    ny = 8
  [../]
  [./gpd]
    type = MeshGeneratorPD
    input = gmg
    retain_fe_mesh = false
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
  [./critical_stress]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./bond_status]
    type = RankTwoBasedFailureCriteriaNOSPD
    variable = bond_status
    rank_two_tensor = stress
    critical_variable = critical_stress
    failure_criterion = VonMisesStress
  [../]
[]

[UserObjects]
  [./shape_singularity]
    type = SingularShapeTensorEliminatorUserObjectPD
  [../]
[]

[ICs]
  [./critical_stretch]
    type = ConstantIC
    variable = critical_stress
    value = 150
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1003
    value = 0.0
  [../]
  [./top_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1002
    value = 0.0
  [../]
  [./bottom_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 1000
    function = '-0.002*t'
  [../]

  [./rbm_x]
    type = RBMPresetOldValuePD
    variable = disp_x
    boundary = 999
  [../]
  [./rbm_y]
    type = RBMPresetOldValuePD
    variable = disp_y
    boundary = 999
  [../]
[]

[Modules/Peridynamics/Mechanics/Master]
  [./all]
    formulation = NONORDINARY_STATE
    stabilization = BOND_HORIZON_I
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2e5
    poissons_ratio = 0.33
  [../]
  [./strain]
    type = ComputeSmallStrainNOSPD
    stabilization = BOND_HORIZON_I
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
[]

[Postprocessors]
  [./bond_status_updated_times]
    type = BondStatusConvergedPostprocessorPD
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  line_search = none
  start_time = 0
  dt = 0.5
  end_time = 1

  fixed_point_max_its = 5
  accept_on_max_fixed_point_iteration = true
  custom_pp = bond_status_updated_times
  custom_abs_tol = 2
  disable_picard_residual_norm_check = true

  [./Quadrature]
    type = GAUSS_LOBATTO
    order = FIRST
  [../]
[]

[Outputs]
  file_base = 2D_bond_status_convergence_H1NOSPD
  exodus = true
[]
