[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[MeshModifiers]
  [./cnode]
    type = AddExtraNodeset
    coord = '0.0 0.0 0.0'
    new_boundary = 100
  [../]
[]

[Variables]
  [./u_x]
  [../]
  [./u_y]
  [../]
  [./u_z]
  [../]
  [./global_strain]
    order = SIXTH
    family = SCALAR
  [../]
[]

[AuxVariables]
  [./ug_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./ug_y]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./ug_z]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./disp_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./disp_y]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./disp_z]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./s01]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e01]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./ug_x]
    type = GlobalDisplacementAux
    variable = ug_x
  [../]
  [./ug_y]
    type = GlobalDisplacementAux
    variable = ug_y
    component = 1
  [../]
  [./ug_z]
    type = GlobalDisplacementAux
    variable = ug_z
    component = 2
  [../]
  [./disp_x]
    type = TotalDisplacementAux
    variable = disp_x
    displacement_variables = 'u_x ug_x'
  [../]
  [./disp_y]
    type = TotalDisplacementAux
    variable = disp_y
    displacement_variables = 'u_y ug_y'
  [../]
  [./disp_z]
    type = TotalDisplacementAux
    variable = disp_z
    displacement_variables = 'u_z ug_z'
  [../]
  [./s01]
    type = RankTwoAux
    variable = s01
    rank_two_tensor = stress
    index_i = 0
    index_j = 1
  [../]
  [./e01]
    type = RankTwoAux
    variable = e01
    rank_two_tensor = total_strain
    index_i = 0
    index_j = 1
  [../]
[]

[GlobalParams]
  displacements = 'u_x u_y u_z'
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
    [./all]
      auto_direction = 'x y z'
      variable = ' u_x u_y u_z'
    [../]
  [../]

  # fix center point location
  [./centerfix_x]
    type = PresetBC
    boundary = 100
    variable = u_x
    value = 0
  [../]
  [./centerfix_y]
    type = PresetBC
    boundary = 100
    variable = u_y
    value = 0
  [../]
  [./centerfix_z]
    type = PresetBC
    boundary = 100
    variable = u_z
    value = 0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    C_ijkl = '70e9 0.33'
    fill_method = symmetric_isotropic_E_nu
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
    applied_stress_tensor = '0 0 0 5e9 5e9 5e9'
    execute_on = 'Initial Timestep_begin Linear'
  [../]
[]

[Postprocessors]
  [./l2err_e01]
    type = ElementL2Error
    variable = e01
    function = -0.095 #Shear strain check
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

  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm         31   preonly   lu      1'

  l_max_its = 30
  nl_max_its = 12

  l_tol = 1.0e-4

  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10

  start_time = 0.0
  num_steps = 2
[]

[Outputs]
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]
