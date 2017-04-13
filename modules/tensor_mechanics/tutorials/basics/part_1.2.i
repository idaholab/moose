#Tensor Mechanics tutorial: the basics
#Step 1, part 2
#2D simulation of uniaxial tension with linear elasticity with visualized stress

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = necking_quad4.e
  uniform_refine = 1
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[Kernels]
  [./TensorMechanics]
  [../]
[]

[AuxVariables]
  [./stress_xx]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./Von_Mises_stress]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    variable = stress_xx
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
  [../]
  [./Von_Mises_stress]
    type = RankTwoScalarAux
    variable = Von_Mises_stress
    rank_two_tensor = stress
    scalar_type = VonMisesStress
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.1e5
    poissons_ratio = 0.3
  [../]
  [./strain]
    type = ComputeSmallStrain
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
[]

[BCs]
  [./left]
    type = PresetBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./bottom]
    type = PresetBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./top]
    type = PresetBC
    variable = disp_y
    boundary = top
    value = 0.0035
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -sub_pc_type -pc_asm_overlap -ksp_gmres_restart'
  petsc_options_value = 'asm lu 1 101'
[]

[Outputs]
  exodus = true
  print_perf_log = true
[]
