#Tensor Mechanics tutorial: the basics
#Step 2, part 1
#2D axisymmetric RZ simulation of uniaxial tension linear elasticity

[GlobalParams]
  displacements = 'disp_r disp_z'
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = necking_quad4.e
  uniform_refine = 1
[]

[Variables]
  [./disp_r]
  [../]
  [./disp_z]
  [../]
[]

[Kernels]
  [./TensorMechanics]
  [../]
[]

[AuxVariables]
  [./stress_tt]
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
    variable = stress_tt
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
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
    type = ComputeAxisymmetricRZSmallStrain
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
[]

[BCs]
  [./left]
    type = PresetBC
    variable = disp_r
    boundary = left
    value = 0.0
  [../]
  [./bottom]
    type = PresetBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  [../]
  [./top]
    type = PresetBC
    variable = disp_z
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
