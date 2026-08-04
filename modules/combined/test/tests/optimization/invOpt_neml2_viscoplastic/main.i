[Optimization]
[]

[OptimizationReporter]
  type = GeneralOptimization
  objective_name = obj_value
  parameter_names = 'E sigma_y C eta'
  num_values = '1 1 1 1'
  # Parameters are NORMALIZED by their own starting guess, so every one starts at
  # 1 and the physical guess lives in the Function scales in forward_and_adjoint.i
  # (8.0e4, 85, 8.0e3, 400). Physically all four start below truth, so all four
  # gradients start negative; starting AT truth zeroes every gradient and makes a
  # standalone run look broken. Truth in normalized units is 1.25 / 1.176 / 1.5 / 1.25.
  initial_condition = '1; 1; 1; 1'
  # Bounds are load-bearing, not cosmetic. The Perzyna flow rate is (f/eta)^n,
  # so a step to eta <= 0 gives a NaN, and because the sub-app pins dtmin that
  # becomes a hard failure rather than a time step cut. Do not remove.
  lower_bounds = '0.1; 0.1; 0.1; 0.1'
  upper_bounds = '10; 10; 10; 10'
[]

[Executioner]
  type = Optimize
  tao_solver = taobqnls
  # -tao_ls_type is deliberately not set. In physical units this problem is badly
  # scaled - parameters 1e2 to 1e5, gradients 1e-9 to 1e-6, a 3913x spread - and
  # that unscaled form made no progress under 'unit'. Normalizing the parameters
  # fixes the conditioning, but the default line search is still what is measured
  # here. The sibling invOpt_elasticity_modular sets 'unit' for its own,
  # better-scaled problem; do not strip it from there.
  #
  # -tao_gatol 1e-12 is load-bearing, NOT paranoia. It is the criterion that
  # actually binds (final gnorm ~6e-13). The gold holds four gradient columns
  # which at convergence are noise near 1e-13; CSVDiff's abs_zero of 1e-10 then
  # treats them as exact zero. Loosening gatol lifts them above abs_zero, where
  # they get compared with rel_err and the test goes platform-dependent.
  # -tao_max_it is a guard rail only; convergence takes about 23 iterations.
  petsc_options_iname = '-tao_gatol -tao_max_it'
  petsc_options_value = '1e-12 50'
[]

[MultiApps]
  # TransientAndAdjoint runs its adjoint sweep inside the same app execution,
  # so one FORWARD execution yields both the objective and all four gradients.
  [forward]
    type = FullSolveMultiApp
    input_files = forward_and_adjoint.i
    execute_on = 'FORWARD'
  []
[]

[Transfers]
  [toForward]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/E
                      OptimizationReporter/sigma_y
                      OptimizationReporter/C
                      OptimizationReporter/eta'
    to_reporters = 'params/E
                    params/sigma_y
                    params/C
                    params/eta'
  []
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'measure_data/objective_value
                      grad_E/inner_product
                      grad_sigma_y/inner_product
                      grad_C/inner_product
                      grad_eta/inner_product'
    to_reporters = 'OptimizationReporter/obj_value
                    OptimizationReporter/grad_E
                    OptimizationReporter/grad_sigma_y
                    OptimizationReporter/grad_C
                    OptimizationReporter/grad_eta'
  []
[]

[Reporters]
  [optInfo]
    type = OptimizationInfo
    items = 'current_iterate function_value gnorm'
  []
[]

[Outputs]
  console = false
  csv = true
[]
