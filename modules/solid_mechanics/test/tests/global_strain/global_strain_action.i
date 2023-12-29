[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    xmin = -0.5
    xmax = 0.5
    ymin = -0.5
    ymax = 0.5
    zmin = -0.5
    zmax = 0.5
  []
  [cnode]
    type = ExtraNodesetGenerator
    coord = '0 -0.5 0'
    new_boundary = 100
    input = generated_mesh
  []
[]

[GlobalParams]
  displacements = 'u_x u_y u_z'
  block = 0
[]

[Variables]
  [./global_strain]
    order = SIXTH
    family = SCALAR
  [../]
[]

[Modules]
  [./TensorMechanics]
    # Master action for generating the tensor mechanics kernels, variables,
    # strain calculation material, and the auxilliary system for visualization
    [./Master]
      [./stress_div]
        strain = SMALL
        add_variables = true
        global_strain = global_strain #global strain contribution
        generate_output = 'strain_xx strain_xy strain_yy stress_xx stress_xy
                           stress_yy vonmises_stress'
      [../]
    [../]
    # GlobalStrain action for generating the objects associated with the global
    # strain calculation and associated displacement visualization
    [./GlobalStrain]
      [./global_strain]
        scalar_global_strain = global_strain
        displacements = 'u_x u_y u_z'
        auxiliary_displacements = 'disp_x disp_y disp_z'
        global_displacements = 'ug_x ug_y ug_z'
      [../]
    [../]
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'z'
      variable = 'u_x u_y u_z'
    [../]
  [../]

  # fix center point location
  [./centerfix_x]
    type = DirichletBC
    boundary = 100
    variable = u_x
    value = 0
  [../]
  [./centerfix_z]
    type = DirichletBC
    boundary = 100
    variable = u_z
    value = 0
  [../]
  # applied displacement
  [./appl_y]
    type = DirichletBC
    boundary = top
    variable = u_y
    value = 0.033
  [../]
  [./fix_y]
    type = DirichletBC
    boundary = bottom
    variable = u_y
    value = 0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    C_ijkl = '7 0.33'
    fill_method = symmetric_isotropic_E_nu
  [../]
  [./stress]
    type = ComputeLinearElasticStress
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

  nl_rel_tol = 1.0e-6
  nl_abs_tol = 1.0e-10

  start_time = 0.0
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
