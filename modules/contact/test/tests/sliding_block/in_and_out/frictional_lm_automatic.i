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

[Modules/TensorMechanics/Master]
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
    youngs_modulus = 1e11
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

[Debug]
  show_var_residual_norms = true
[]

[Postprocessors]
  [nonlinear_its]
    type = NumNonlinearIterations
    execute_on = timestep_end
  []
  [tot_nonlin_it]
    type = CumulativeValuePostprocessor
    postprocessor = nonlinear_its
  []
  [linear_its]
    type = NumLinearIterations
    execute_on = timestep_end
  []
  [tot_lin_it]
    type = CumulativeValuePostprocessor
    postprocessor = linear_its
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  # petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_ksp_ew -pc_svd_monitor'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -mat_mffd_err -pc_factor_shift_type '
                        '-pc_factor_shift_amount '
  petsc_options_value = 'lu        superlu_dist                  1e-5          NONZERO               '
                        '1e-10'

  end_time = 0.15
  dt = 0.01
  dtmin = 0.01
  l_max_its = 30
  nl_max_its = 20
  line_search = 'none'
  timestep_tolerance = 1e-6
  snesmf_reuse_base = false
[]

[Outputs]
  exodus = true
[]

[Functions]
  [vertical_movement]
    type = ParsedFunction
    expression = -0.05*t
  []
  [horizontal_movement]
    type = ParsedFunction
    expression = -0.04*sin(8*t)+0.02
  []
[]

[Contact]
  [contact]
    secondary = 3
    primary = 2
    model = coulomb
    formulation = mortar
    automatic_c = true
    c_normal = 1e+5
    c_tangential = 1e+10
    friction_coefficient = 0.4
    tangential_lm_scaling = 1.0e-10
    correct_edge_dropping = true
    interpolate_normals = false
    # normal_lm_scaling = 1.0e-10
  []
[]

[Postprocessors]
  [contact]
    type = ContactDOFSetSize
    variable = contact_normal_lm
    subdomain = 'contact_secondary_subdomain'
    execute_on = 'nonlinear timestep_end'
  []
[]
