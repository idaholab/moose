[Mesh]
  type = FileMesh
  file = 'L.exo'
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = true
  stabilize_strain = true
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Functions]
  [pfn]
    type = PiecewiseLinear
    x = '0    1    2'
    y = '0.00 0.3 0.5'
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
  [left]
    type = DirichletBC
    preset = true
    variable = disp_x
    boundary = fix
    value = 0.0
  []

  [bottom]
    type = DirichletBC
    preset = true
    variable = disp_y
    boundary = fix
    value = 0.0
  []

  [back]
    type = DirichletBC
    preset = true
    variable = disp_z
    boundary = fix
    value = 0.0
  []

  [front]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = pull
    function = pfn
    preset = true
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
  [cauchy_stress_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [cauchy_stress_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [cauchy_stress_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [cauchy_stress_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [cauchy_stress_yz]
    order = CONSTANT
    family = MONOMIAL
  []
  [cauchy_stress_xz]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [cauchy_stress_xx]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = cauchy_stress_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  []
  [cauchy_stress_yy]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = cauchy_stress_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  []
  [cauchy_stress_zz]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = cauchy_stress_zz
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  []
  [cauchy_stress_xy]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = cauchy_stress_xy
    index_i = 0
    index_j = 1
    execute_on = timestep_end
  []
  [cauchy_stress_xz]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = cauchy_stress_xz
    index_i = 0
    index_j = 2
    execute_on = timestep_end
  []
  [cauchy_stress_yz]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = cauchy_stress_yz
    index_i = 1
    index_j = 2
    execute_on = timestep_end
  []

  [strain_xx]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  []
  [strain_yy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  []
  [strain_zz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_zz
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  []
  [strain_xy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xy
    index_i = 0
    index_j = 1
    execute_on = timestep_end
  []
  [strain_xz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xz
    index_i = 0
    index_j = 2
    execute_on = timestep_end
  []
  [strain_yz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_yz
    index_i = 1
    index_j = 2
    execute_on = timestep_end
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 100000.0
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

  solve_type = 'newton'

  petsc_options_iname = -pc_type
  petsc_options_value = lu

  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-8

  end_time = 1.0
  dtmin = 0.5
  dt = 0.5
[]

[Outputs]
  exodus = true
  csv = false
[]
