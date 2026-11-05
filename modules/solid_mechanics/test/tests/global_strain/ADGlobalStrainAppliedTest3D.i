[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 6
    ny = 6
    nz = 6
    xmin = -0.5
    xmax = 0.5
    ymin = -0.5
    ymax = 0.5
    zmin = -0.5
    zmax = 0.5
  []
  [Anode]
    type = ExtraNodesetGenerator
    coord = '0 0 0'
    new_boundary = 100
    use_closest_node = true
    input = generated_mesh
  []
  [Bnode]
    type = ExtraNodesetGenerator
    coord = '.333 -.167 -.333'
    new_boundary = 101
    use_closest_node = true
    input = Anode
  []
  [Cnode]
    type = ExtraNodesetGenerator
    coord = '.167 .333 .25'
    new_boundary = 102
    use_closest_node = true
    input = Bnode
  []
[]

[AuxVariables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[GlobalParams]
  displacements = 'u_x u_y u_z'
[]

[Outputs]
  exodus = true
[]

[Variables]
  [GS_diag]
    order = THIRD
    family = SCALAR
  []
  [GS_off_diag]
    order = THIRD
    family = SCALAR
  []
  [dummy] # Used for command line options for the test harness. Has no affect on the solve.
  []
[]

[BCs]
  [Periodic]
    [all]
      auto_direction = 'x y z'
    []
  []
  [A1]
    type = DirichletBC
    boundary = 100
    variable = u_x
    value = 0
  []
  [A2]
    type = DirichletBC
    boundary = 100
    variable = u_y
    value = 0
  []
  [A3]
    type = DirichletBC
    boundary = 100
    variable = u_z
    value = 0
  []
  [B1]
    type = DirichletBC
    boundary = 101
    value = 0
    variable = dummy
  []
  [B2]
    type = DirichletBC
    boundary = 101
    value = 0
    variable = dummy
  []
  [C1]
    type = DirichletBC
    boundary = 102
    value = 0
    variable = dummy
  []
[]

[AuxKernels]
  [disp_x]
    type = ADGlobalDisplacementAux
    component = 0
    diagonal_global_strain = GS_diag
    off_diagonal_global_strain = GS_off_diag
    displacements = 'u_x u_y u_z'
    global_strain_uo = 'GS_peri_dir'
    variable = disp_x
  []
  [disp_y]
    type = ADGlobalDisplacementAux
    component = 1
    diagonal_global_strain = GS_diag
    off_diagonal_global_strain = GS_off_diag
    displacements = 'u_x u_y u_z'
    global_strain_uo = 'GS_peri_dir'
    variable = disp_y
  []
  [disp_z]
    type = ADGlobalDisplacementAux
    component = 2
    diagonal_global_strain = GS_diag
    off_diagonal_global_strain = GS_off_diag
    displacements = 'u_x u_y u_z'
    global_strain_uo = 'GS_peri_dir'
    variable = disp_z
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        add_variables = true
        strain = SMALL
        incremental = false
        global_strain = global_strain
        use_automatic_differentiation = true
        automatic_eigenstrain_names = false
      []
    []
  []
[]

[Kernels]
  [GS_diag_kernel]
    type = ADGlobalStrain
    component_type = DIAGONAL
    global_strain_uo = GS_peri_dir
    scalar_global_strain = GS_diag
    applied_stress_tensor = '.05 .065 .06 .05 .05 .05'
    variable = u_x
  []
  [GS_off_diag_kernel]
    type = ADGlobalStrain
    component_type = OFF_DIAGONAL
    global_strain_uo = GS_peri_dir
    applied_stress_tensor = '.05 .065 .06 .05 .05 .05'
    scalar_global_strain = GS_off_diag
    variable = u_x
  []
  [dummy]
    type = TimeDerivative
    variable = dummy
  []
[]

[UserObjects]
  [GS_peri_dir]
    type = GlobalStrainPeriodicDirUserObject
    displacements = 'u_x u_y u_z'
  []
[]

[Materials]
  [elastic_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1
    poissons_ratio = .2
  []
  [global_strain_mat]
    type = ADComputeGlobalStrain
    diagonal_global_strain = GS_diag
    off_diagonal_global_strain = GS_off_diag
    displacements = 'u_x u_y u_z'
    global_strain_uo = GS_peri_dir
  []
  [stress]
    type = ADComputeLinearElasticStress
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = 'NEWTON'

  line_search = basic
  abort_on_solve_fail = true
  petsc_options_iname = ' -pc_type   -pc_factor_shift_type'
  petsc_options_value = '       lu            NONZERO'

  l_max_its = 30
  nl_max_its = 10

  l_tol = 1.0e-4

  nl_rel_tol = 1.0e-20
  nl_abs_tol = 1.0e-15

  start_time = 0.0
  num_steps = 1
  dt = 1
[]
