[Mesh]
  [fmesh]
    type = CartesianMeshGenerator
    dim = 3
    dx = 1.0
    dy = 1.0
    dz = 1.0
  []
[]

[Problem]
  kernel_coverage_check = false
  material_coverage_check = false
[]

[Debug]
  show_material_props = true
[]

[Variables]
  [u]
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       30                 NONZERO               1e-10   '

  dt = 0.1
  end_time = 1.0
  #[TimeStepper]
  #  type = IterationAdaptiveDT
  #  dt = 0.1
  #  iteration_window = 2
  #  optimal_iterations = 6
  #  growth_factor = 1.25
  #  cutback_factor = 0.8
  #[]

  nl_abs_tol = 1e-1
  nl_max_its = 30

  fixed_point_max_its = 30
  fixed_point_min_its = 1
  fixed_point_rel_tol = 1e-6
[]

[MultiApps]
  [subchannel]
    type = TransientMultiApp
    input_files = fuel_assembly.i
    positions = '0 0 0'
    max_procs_per_app = 1
    execute_on = TIMESTEP_END
  []
[]

[Outputs]
  exodus = true
  #print_linear_residuals = false
[]
