# InputFileFormatter

This class produces produces a dump of the `InputParameters` that appears like the normal input
file syntax.

This formatter is used for the `--show-input` command line option, described
[here](modules/doc/content/application_usage/command_line_usage.md optional=True). It is also available for the
application developer, either directly or using an [AdvancedOutput](syntax/Outputs/index.md#advanced-output).

## Example output

The snippet below shows the output for `--show-input` for the boundary condition and executioner
blocks of the `simple_diffusion` test, shown further below. We can see additional metadata about each object,
some default parameters that are not explicitly written in the input files, as well as
disabled invalid parameters.

```
[BCs]

  [./left]
    boundary                     = left
    control_tags                 = INVALID
    displacements                = INVALID
    enable                       = 1
    extra_matrix_tags            = INVALID
    extra_vector_tags            = INVALID
    implicit                     = 1
    inactive                     = (no_default)
    isObjectAction               = 1
    matrix_tags                  = system
    seed                         = 0
    type                         = DirichletBC
    use_displaced_mesh           = 0
    variable                     = u
    vector_tags                  = nontime
    diag_save_in                 = INVALID
    preset                       = 1
    save_in                      = INVALID
    value                        = 0
  [../]

  [./right]
    boundary                     = right
    control_tags                 = INVALID
    displacements                = INVALID
    enable                       = 1
    extra_matrix_tags            = INVALID
    extra_vector_tags            = INVALID
    implicit                     = 1
    inactive                     = (no_default)
    isObjectAction               = 1
    matrix_tags                  = system
    seed                         = 0
    type                         = DirichletBC
    use_displaced_mesh           = 0
    variable                     = u
    vector_tags                  = nontime
    diag_save_in                 = INVALID
    preset                       = 1
    save_in                      = INVALID
    value                        = 1
  [../]
[]

[Executioner]
  auto_preconditioning           = 1
  inactive                       = (no_default)
  isObjectAction                 = 1
  type                           = Steady
  accept_on_max_fixed_point_iteration = 0
  accept_on_max_picard_iteration = 0
  auto_advance                   = INVALID
  automatic_scaling              = INVALID
  compute_initial_residual_before_preset_bcs = 0
  compute_scaling_once           = 1
  contact_line_search_allowed_lambda_cuts = 2
  contact_line_search_ltol       = INVALID
  control_tags                   = (no_default)
  custom_abs_tol                 = 1e-50
  custom_pp                      = INVALID
  custom_rel_tol                 = 1e-08
  direct_pp_value                = 0
  disable_fixed_point_residual_norm_check = 0
  disable_picard_residual_norm_check = 0
  enable                         = 1
  fixed_point_abs_tol            = 1e-50
  fixed_point_algorithm          = picard
  fixed_point_force_norms        = 0
  fixed_point_max_its            = 1
  fixed_point_min_its            = 1
  fixed_point_rel_tol            = 1e-08
  l_abs_tol                      = 1e-50
  l_max_its                      = 10000
  l_tol                          = 1e-05
  line_search                    = default
  line_search_package            = petsc
  max_xfem_update                = 4294967295
  mffd_type                      = wp
  n_max_nonlinear_pingpong       = 100
  nl_abs_div_tol                 = 1e+50
  nl_abs_step_tol                = 0
  nl_abs_tol                     = 1e-50
  nl_div_tol                     = 1e+10
  nl_forced_its                  = 0
  nl_max_funcs                   = 10000
  nl_max_its                     = 50
  nl_rel_step_tol                = 0
  nl_rel_tol                     = 1e-08
  num_grids                      = 1
  off_diagonals_in_auto_scaling  = 0
  outputs                        = INVALID
  petsc_options                  = INVALID
  petsc_options_iname            = '-pc_type -pc_hypre_type'
  petsc_options_value            = 'hypre boomeramg'
  picard_abs_tol                 = 1e-50
  picard_custom_pp               = INVALID
  picard_force_norms             = 0
  picard_max_its                 = 1
  picard_rel_tol                 = 1e-08
  relaxation_factor              = 1
  relaxed_variables              = (no_default)
  resid_vs_jac_scaling_param     = 0
  restart_file_base              = (no_default)
  scaling_group_variables        = INVALID
  skip_exception_check           = 0
  snesmf_reuse_base              = 1
  solve_type                     = PJFNK
  splitting                      = INVALID
  time                           = 0
  transformed_postprocessors     = (no_default)
  transformed_variables          = (no_default)
  update_xfem_at_timestep_begin  = 0
  verbose                        = 0
[]
```

!listing moose/test/tests/kernels/simple_diffusion/simple_diffusion.i caption=Input file used to show to formatting
