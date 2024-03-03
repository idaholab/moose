nz = 1
z = '${fparse nz*0.2}'

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = true
  stabilize_strain = true
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 5
    ny = 5
    nz = ${nz}
    zmax = ${z}
  []
  use_displaced_mesh = false
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Kernels]
  [sdx]
    type = TotalLagrangianStressDivergence
    variable = disp_x
    component = 0
  []
  [sdy]
    type = TotalLagrangianStressDivergence
    variable = disp_y
    component = 1
  []
  [sdz]
    type = TotalLagrangianStressDivergence
    variable = disp_z
    component = 2
  []
[]

[BCs]
  [fix_x]
    type = DirichletBC
    boundary = 'top bottom'
    variable = disp_x
    value = 0
  []
  [fix_y]
    type = DirichletBC
    boundary = 'bottom'
    variable = disp_y
    value = 0
  []
  [fix_z]
    type = DirichletBC
    boundary = 'top bottom'
    variable = disp_z
    value = 0
  []
  [disp_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'top'
    function = 't'
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1000.0
    poissons_ratio = 0.25
  []
  [strain]
    type = ComputeLagrangianStrain
  []
  [stress]
    type = ComputeLagrangianLinearElasticStress
  []
  [stress_zz]
    type = RankTwoCartesianComponent
    rank_two_tensor = cauchy_stress
    index_i = 2
    index_j = 2
    property_name = stress_zz
  []
  [strain_zz]
    type = RankTwoCartesianComponent
    rank_two_tensor = mechanical_strain
    index_i = 2
    index_j = 2
    property_name = strain_zz
  []
[]

[Executioner]
  type = Transient
  dt = 0.01
  end_time = 0.1

  solve_type = 'newton'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-6
[]

[Postprocessors]
  [strain_zz]
    type = ElementAverageMaterialProperty
    mat_prop = strain_zz
  []
  [stress_zz]
    type = ElementAverageMaterialProperty
    mat_prop = stress_zz
  []
[]

[Outputs]
  csv = true
  file_base = 'pull_3D_nz_${nz}'
[]
