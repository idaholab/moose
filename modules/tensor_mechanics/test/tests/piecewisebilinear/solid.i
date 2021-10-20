#mechanics

[Mesh]
  file = block_test.e
  use_displaced_mesh = false
[]
#############################################################
[GlobalParams]
  displacements = 'disp_i disp_j disp_k'
[]
############################################################
[Variables]
  [./disp_i]
    scaling = 1E-10
  [../]

  [./disp_j]
    scaling = 1E-10
  [../]

  [./disp_k]
    scaling = 1E-10
  [../]
[]

############################################################
[Kernels]
  [./grad_stress_x]
    type = StressDivergenceTensors
    variable = disp_i
    component = 0
  [../]

  [./grad_stress_y]
    type = StressDivergenceTensors
    variable = disp_j
    component = 1
  [../]

  [./grad_stress_z]
    type = StressDivergenceTensors
    variable = disp_k
    component = 2
  [../]
[]
###########################################################
[AuxVariables]
  [./stress_ii]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./stress_ij]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./stress_ik]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./stress_jj]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./stress_jk]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./stress_kk]
    order = CONSTANT
    family = MONOMIAL
  [../]

[]

#********************************************************
[Functions]
# displacement on boundary surfaces

  [./Ux_on_face_X_Plus]
    type = PiecewiseBilinear
    data_file = X_Plus_2x2_block.csv
    xaxis = 2
    yaxis = 1
  [../]

[] 

##############################################################
[AuxKernels]
  [./stress_ii]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_ii
    index_i = 0
    index_j = 0
  [../]

  [./stress_ij]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_ij
    index_i = 0
    index_j = 1
  [../]

  [./stress_ik]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_ik
    index_i = 0
    index_j = 2
  [../]

  [./stress_jj]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_jj
    index_i = 1
    index_j = 1
  [../]

  [./stress_jk]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_jk
    index_i = 1
    index_j = 2
  [../]

  [./stress_kk]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_kk
    index_i = 2
    index_j = 2
  [../]

[]
############################################################
[BCs]

 # B.C. for solid field
   # Prescribed displacements at three boundary surfaces

  [./roller_sigma_h_min]
    type = DirichletBC
    preset = true
    variable = disp_i
    value = 0
    boundary = 'Side_X_Minus'
  [../]

  [./roller_sigma_h_max]
    type = DirichletBC
    preset = true
    variable = disp_j
    value = 0
    boundary = 'Side_Y_Minus'
  [../]

  [./roller_signma_v]
    type = DirichletBC
    preset = true
    variable = disp_k
    value = 0
    boundary = 'Side_Z_Minus'
  [../]

  # apply linear displacement 
  [./Side_X_Plus_T]
    type = FunctionDirichletBC
    variable = disp_i
    boundary = 'Side_X_Plus'
    function = Ux_on_face_X_Plus
  [../]

[]
############################################################
[Materials]

#mechanics

  [./elasticity_sediment]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 3.0e+10
    poissons_ratio = 0.3
    block = 1
  [../]

  [./strain]
    #type = ComputeIncrementalSmallStrain
    type = ComputeSmallStrain
  [../]

  [./stress]
    type = ComputeLinearElasticStress
  [../]
[]
############################################################
[Preconditioning]
active = 'ilu_may_use_less_mem'
#active = 'preferred'
#active = 'superlu'

  [./ilu_may_use_less_mem]
    type = SMP
    full = true
    petsc_options = '-ksp_diagonal_scale -ksp_diagonal_scale_fix'
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type'
    petsc_options_value = 'gmres asm ilu NONZERO'
  [../]

  [./superlu]
    type = SMP
    full = true
    petsc_options = '-ksp_diagonal_scale -ksp_diagonal_scale_fix'
    petsc_options_iname = '-ksp_type -pc_type -pc_factor_mat_solver_package'
    petsc_options_value = 'gmres lu superlu_dist'
  [../]

  [./preferred]
    #per Andy
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
    petsc_options_value = ' lu       mumps'
  [../]
[]
###########################################################
[Executioner]
  type = Steady
  solve_type = Newton

  #   end_time = 1.0
  #   dt = 0.1

  l_max_its  = 10
  l_tol      = 1e-4
  nl_max_its = 100

  nl_abs_tol = 1e-4
  nl_rel_tol = 1e-4
[]
############################################################
[Outputs]
  execute_on = 'timestep_end'
  exodus  = true

  [./console]
    type = Console
    output_linear = true
    output_nonlinear = true
    verbose = true
  [../]

  [pgraph]
    type = PerfGraphOutput
    execute_on = 'final'  # Default is "final"
    level = 2                     # Default is 1
    heaviest_branch = true        # Default is false
    heaviest_sections = 5         # Default is 0
  [../]

  [./CSV]
    type = CSV
  [../]
[]
############################################################
