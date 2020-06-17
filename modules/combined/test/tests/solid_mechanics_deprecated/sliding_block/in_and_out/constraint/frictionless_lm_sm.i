[Mesh]
  patch_size = 80
  [file]
    type = FileMeshGenerator
    file = sliding_elastic_blocks_2d.e
  []
  [secondary]
    input = file
    type = LowerDBlockFromSidesetGenerator
    sidesets = '3'
    new_block_id = '30'
  []
  [primary]
    input = secondary
    type = LowerDBlockFromSidesetGenerator
    sidesets = '2'
    new_block_id = '20'
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = false
[]

[Variables]
  [./disp_x]
    block = '1 2'
  [../]
  [./disp_y]
    block = '1 2'
  [../]
  [normal_lm]
    block = '30'
  []
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./left_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./right_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 4
    function = horizontal_movement
  [../]
  [./right_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 4
    function = vertical_movement
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    block = '1 2'
  [../]
[]

[Materials]
  [./left]
    type = Elastic
    formulation = NonlinearPlaneStrain
    block = 1
    disp_y = disp_y
    disp_x = disp_x
    poissons_ratio = 0.3
    youngs_modulus = 1e6
  [../]
  [./right]
    type = Elastic
    formulation = NonlinearPlaneStrain
    block = 2
    disp_y = disp_y
    disp_x = disp_x
    poissons_ratio = 0.3
    youngs_modulus = 1e6
  [../]
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options = '-snes_converged_reason -ksp_converged_reason'
  petsc_options_iname = '-pc_type -mat_mffd_err -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       1e-5          NONZERO               1e-15'
  end_time = 15
  dt = 0.1
  dtmin = 0.01
  l_max_its = 30
  nl_max_its = 20
  line_search = 'none'
  timestep_tolerance = 1e-6
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  sync_times = '1 2 3 4 5 6 7 8 9 10 11 12 13 14 15'
  [out]
    type = Exodus
    sync_only = true
  []
  [dof]
    execute_on = 'initial'
    type = DOFMap
  []
  [csv]
    type = CSV
    execute_on = 'nonlinear timestep_end'
  []
[]

[Functions]
  [./vertical_movement]
    type = ParsedFunction
    value = -t
  [../]
  [./horizontal_movement]
    type = ParsedFunction
    value = -0.04*sin(4*t)+0.02
  [../]
[]

[Constraints]
  [./lm]
    type = NormalNodalLMMechanicalContact
    secondary = 3
    primary = 2
    variable = normal_lm
    primary_variable = disp_x
    disp_y = disp_y
    ncp_function_type = min
    use_displaced_mesh = true
  [../]
  [normal_x]
    type = NormalMortarMechanicalContact
    primary_boundary = '2'
    secondary_boundary = '3'
    primary_subdomain = '20'
    secondary_subdomain = '30'
    variable = normal_lm
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
  []
  [normal_y]
    type = NormalMortarMechanicalContact
    primary_boundary = '2'
    secondary_boundary = '3'
    primary_subdomain = '20'
    secondary_subdomain = '30'
    variable = normal_lm
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
  []
[]

[Postprocessors]
  [./num_nl]
    type = NumNonlinearIterations
  [../]
  [lin]
    type = NumLinearIterations
  []
  [contact]
    type = ContactDOFSetSize
    variable = normal_lm
    subdomain = '30'
    execute_on = 'nonlinear timestep_end'
  []
[]
