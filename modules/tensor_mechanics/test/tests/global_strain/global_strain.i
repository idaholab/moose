[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
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
[]

[Variables]
  [./u_x]
  [../]
  [./u_y]
  [../]
  [./global_strain]
    order = THIRD
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
  [./disp_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./disp_y]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./ug_x]
    type = GlobalDisplacementAux
    variable = disp_x
  [../]
  [./ug_y]
    type = GlobalDisplacementAux
    variable = disp_y
    component = 1
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
[]

[GlobalParams]
  displacements = 'u_x u_y'
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
      auto_direction = 'x y'
      variable = 'u_x u_y'
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
    applied_stress_tensor = '0.1 0.2 0 0 0 -0.2'
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
