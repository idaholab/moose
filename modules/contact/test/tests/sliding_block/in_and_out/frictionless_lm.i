[Mesh]
  patch_size = 80
  [file]
    type = FileMeshGenerator
    file = sliding_elastic_blocks_2d.e
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = false
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = true
    strain = FINITE
    block = '1 2'
  []
[]

[BCs]
  [left_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  []
  [left_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  []
  [right_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 4
    function = horizontal_movement
  []
  [right_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 4
    function = vertical_movement
  []
[]

[Materials]
  [left]
    type = ComputeIsotropicElasticityTensor
    block = '1 2'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
    constant_on = SUBDOMAIN
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  []
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
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_ksp_ew'
  petsc_options_iname = '-pc_type -mat_mffd_err -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       1e-5          NONZERO               1e-15'
  end_time = 15
  dt = 0.1
  dtmin = 0.01
  l_max_its = 30
  nl_max_its = 20
  line_search = 'none'
  timestep_tolerance = 1e-6
  snesmf_reuse_base = false
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
  [vertical_movement]
    type = ParsedFunction
    expression = -t
  []
  [horizontal_movement]
    type = ParsedFunction
    expression = -0.04*sin(4*t)+0.02
  []
[]

[Contact]
  [contact]
    secondary = 3
    primary = 2
    model = frictionless
    formulation = mortar
  []
[]

[Postprocessors]
  [num_nl]
    type = NumNonlinearIterations
  []
  [lin]
    type = NumLinearIterations
  []
  [contact]
    type = ContactDOFSetSize
    variable = contact_normal_lm
    subdomain = '30'
    execute_on = 'nonlinear timestep_end'
  []
[]
