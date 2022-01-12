[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = true
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
    type = UpdatedLagrangianStressDivergence
    variable = disp_x
    component = 0
    use_displaced_mesh = true
  []
  [sdy]
    type = UpdatedLagrangianStressDivergence
    variable = disp_y
    component = 1
    use_displaced_mesh = true
  []
  [sdz]
    type = UpdatedLagrangianStressDivergence
    variable = disp_z
    component = 2
    use_displaced_mesh = true
  []
[]

[AuxVariables]
  [strain_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_xz]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_yz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xz]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [stress_xx]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  []
  [stress_yy]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  []
  [stress_zz]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_zz
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  []
  [stress_xy]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_xy
    index_i = 0
    index_j = 1
    execute_on = timestep_end
  []
  [stress_xz]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_xz
    index_i = 0
    index_j = 2
    execute_on = timestep_end
  []
  [stress_yz]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_yz
    index_i = 1
    index_j = 2
    execute_on = timestep_end
  []

  [strain_xx]
    type = RankTwoAux
    rank_two_tensor = mechanical_strain
    variable = strain_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  []
  [strain_yy]
    type = RankTwoAux
    rank_two_tensor = mechanical_strain
    variable = strain_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  []
  [strain_zz]
    type = RankTwoAux
    rank_two_tensor = mechanical_strain
    variable = strain_zz
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  []
  [strain_xy]
    type = RankTwoAux
    rank_two_tensor = mechanical_strain
    variable = strain_xy
    index_i = 0
    index_j = 1
    execute_on = timestep_end
  []
  [strain_xz]
    type = RankTwoAux
    rank_two_tensor = mechanical_strain
    variable = strain_xz
    index_i = 0
    index_j = 2
    execute_on = timestep_end
  []
  [strain_yz]
    type = RankTwoAux
    rank_two_tensor = mechanical_strain
    variable = strain_yz
    index_i = 1
    index_j = 2
    execute_on = timestep_end
  []
[]

[Functions]
  [shearme]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 2'
  []
[]

[BCs]
  [back]
    type = DirichletBC
    preset = true
    variable = disp_z
    boundary = back
    value = 0.0
  []
  [bottom_y]
    type = DirichletBC
    preset = true
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [bottom_x]
    type = DirichletBC
    preset = true
    variable = disp_x
    boundary = bottom
    value = 0.0
  []
  [shear]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = top
    function = shearme
    preset = true
  []
  [hmm]
    type = DirichletBC
    preset = true
    variable = disp_y
    boundary = top
    value = 0.0
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1000.0
    poissons_ratio = 0.25
  []
  [compute_stress]
    type = ComputeLagrangianLinearElasticStress
  []
  [compute_strain]
    type = ComputeLagrangianStrain
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  dt = 0.01

  solve_type = 'newton'

  petsc_options_iname = -pc_type
  petsc_options_value = lu

  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10

  end_time = 1
[]

[Outputs]
  exodus = true
[]
