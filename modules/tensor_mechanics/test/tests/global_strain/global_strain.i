[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
[]

[MeshModifiers]
  [./cnode]
    type = AddExtraNodeset
    coord = '0.0 0.0'
    new_boundary = 100
  [../]
  [./anode]
    type = AddExtraNodeset
    coord = '0.0 0.5'
    new_boundary = 101
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./global_strain]
    order = THIRD
    family = SCALAR
  [../]
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
  block = 0
[]

[Kernels]
  [./TensorMechanics]
  [../]
[]

[ScalarKernels]
  [./global_strain]
    type = GlobalStrain
    variable = global_strain
    global_strain = appl_stress
  [../]
[]

[BCs]
  [./Periodic]
    [./left-right]
      auto_direction = 'y'
      variable = 'disp_x'
    [../]
  [../]

  # fix center point location
  [./centerfix_x]
    type = PresetBC
    boundary = 100
    variable = disp_x
    value = 0
  [../]
  [./centerfix_y]
    type = PresetBC
    boundary = 100
    variable = disp_y
    value = 0
  [../]

  # fix side point x coordinate to inhibit rotation
  [./angularfix]
    type = PresetBC
    boundary = 101
    variable = disp_x
    value = 0
  [../]

  [./applied_disp1]
    type = PresetBC
    boundary = left
    variable = disp_x
    value = -0.2
  [../]

  [./applied_disp2]
    type = PresetBC
    boundary = right
    variable = disp_x
    value = 0.2
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    C_ijkl = '1 1'
    fill_method = symmetric_isotropic
  [../]
  [./strain]
    type = ComputeSmallStrain
    global_strain = global_strain
  [../]
  [./global_strain]
    type = ComputeGlobalStrain
    scalar_global_strain = global_strain
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
[]

[UserObjects]
  [./appl_stress]
    type = GlobalStrainUserObject
    execute_on = 'Initial Timestep_begin Linear'
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
  scheme = bdf2
  solve_type = 'PJFNK'

  line_search = basic

  petsc_options_iname = '-pc_type -sub_pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = ' asm      lu              NONZERO               1e-10'

  l_max_its = 30
  nl_max_its = 12

  l_tol = 1.0e-4

  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10

  start_time = 0.0
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
