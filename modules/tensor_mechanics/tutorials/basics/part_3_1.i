#Tensor Mechanics tutorial: the basics
#Step 3, part 1
#3D simulation of uniaxial tension with J2 plasticity

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  order = second
[]

[Mesh]
  file = necking_quad4.e
  uniform_refine = 0
  second_order = true
[]

[MeshModifiers]
  [./extrude]
    type = MeshExtruder
    extrusion_vector = '0 0 0.5'
    num_layers = 2
    bottom_sideset = 'back'
    top_sideset = 'front'
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
  [../]
[]

[AuxVariables]
  [./stress_zz]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./strain_zz]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./stress_zz]
    type = RankTwoAux
    variable = stress_zz
    rank_two_tensor = stress
    index_i = 1
    index_j = 1
  [../]
  [./strain_zz]
    type = RankTwoAux
    variable = strain_zz
    rank_two_tensor = total_strain
    index_i = 1
    index_j = 1
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.1e5
    poissons_ratio = 0.3
  [../]
  [./strain]
    type = ComputeFiniteStrain
  [../]
  [./stress]
    type = ComputeMultiPlasticityStress
    ep_plastic_tolerance = 1e-9
    plastic_models = J2
  [../]
[]

[UserObjects]
  [./hardening]
    type = TensorMechanicsHardeningCubic
    value_0 = 2.4e2
    value_residual = 3.0e2
    internal_0 = 0
    internal_limit = 0.005
  [../]
  [./J2]
    type = TensorMechanicsPlasticJ2
    yield_strength = hardening
    yield_function_tolerance = 1E-3
    internal_constraint_tolerance = 1E-9
  [../]
[]

[BCs]
  [./left]
    type = PresetBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./back]
    type = PresetBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
  [./bottom]
    type = PresetBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./top]
    type = FunctionPresetBC
    variable = disp_y
    boundary = top
    function = '0.0007*t'
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
  dt = 0.25
  end_time = 16

  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -sub_pc_type -pc_asm_overlap -ksp_gmres_restart'
  petsc_options_value = 'asm lu 1 101'
[]

[Postprocessors]
  [./ave_stress_bottom]
    type = SideAverageValue
    variable = stress_zz
    boundary = bottom
  [../]
  [./ave_strain_bottom]
    type = SideAverageValue
    variable = strain_zz
    boundary = bottom
  [../]
[]

[Outputs]
  exodus = true
  print_perf_log = true
  csv = true
  print_linear_residuals = false
[]
