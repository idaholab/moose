[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 4
    xmin = -0.5
    xmax = 0.5
  []
  [cnode]
    type = ExtraNodesetGenerator
    coord = '0'
    new_boundary = 100
    use_closest_node = true
    input = generated_mesh
  []
[]

[AuxVariables]
  [disp_x]
  []
[]

[GlobalParams]
  displacements = 'u_x'
[]

[Outputs]
  exodus = true
[]

[Variables]
  [GS_diag]
    order = FIRST
    family = SCALAR
  []
[]

[BCs]
  [Periodic]
    [all]
      auto_direction = 'x'
    []
  []
  [centerfix_x]
    type = DirichletBC
    boundary = 100
    variable = u_x
    value = 0
  []
[]

[AuxKernels]
  [disp_x]
    type = ADGlobalDisplacementAux
    component = 0
    diagonal_global_strain = GS_diag
    displacements = 'u_x'
    global_strain_uo = 'GS_peri_dir'
    variable = disp_x
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
    applied_stress_tensor = '.1 .11 .12'
    variable = u_x
  []
[]

[UserObjects]
  [GS_peri_dir]
    type = GlobalStrainPeriodicDirUserObject
    displacements = 'u_x'
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
    displacements = 'u_x'
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
  solve_type = 'PJFNK'

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
