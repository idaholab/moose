[GlobalParams]
  displacements = 'disp_x disp_y'
  large_kinematics = true
  stabilize_strain = true
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  use_displaced_mesh = false
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [strain_zz]
  []
[]

[Kernels]
  [sdx]
    type = TotalLagrangianStressDivergence
    variable = disp_x
    out_of_plane_strain = strain_zz
    component = 0
  []
  [sdy]
    type = TotalLagrangianStressDivergence
    variable = disp_y
    out_of_plane_strain = strain_zz
    component = 1
  []
  [wps]
    type = TotalLagrangianWeakPlaneStress
    variable = strain_zz
  []
[]

[BCs]
  [fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'top bottom'
    value = 0
  []
  [fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
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
    type = ComputeLagrangianWPSStrain
    out_of_plane_strain = strain_zz
  []
  [stress]
    type = ComputeLagrangianLinearElasticStress
  []
[]

[Executioner]
  type = Transient
  dt = 0.01
  end_time = 0.1

  solve_type = 'newton'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10
[]
