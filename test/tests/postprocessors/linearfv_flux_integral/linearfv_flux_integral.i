[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 100
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
    initial_condition = 0.25
  []
[]

[LinearFVKernels]
  [diffusion]
    type = LinearFVDiffusion
    variable = u
    diffusion_coeff = 0.5
    use_nonorthogonal_correction = false
  []
  [advection]
    type = LinearFVAdvection
    variable = u
    velocity = '0.5 0 0'
    advected_interp_method = upwind
  []
[]

[LinearFVBCs]
  [left]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = left
    functor = 1
  []
  [right]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = right
    functor = 0
  []
[]

[Postprocessors]
  [flux_left_total]
    type = BoundaryLinearFVFluxIntegral
    boundary = left
    linearfvkernels = 'diffusion advection'
  []
  [flux_left_diff]
    type = BoundaryLinearFVFluxIntegral
    boundary = left
    linearfvkernels = 'diffusion'
  []
  [flux_left_adv]
    type = BoundaryLinearFVFluxIntegral
    boundary = left
    linearfvkernels = 'advection'
  []
  [flux_right_total]
    type = BoundaryLinearFVFluxIntegral
    boundary = right
    linearfvkernels = 'diffusion advection'
  []
  [flux_right_diff]
    type = BoundaryLinearFVFluxIntegral
    boundary = right
    linearfvkernels = 'diffusion'
  []
  [flux_right_adv]
    type = BoundaryLinearFVFluxIntegral
    boundary = right
    linearfvkernels = 'advection'
  []
  [flux_left_sum_error]
    type = ParsedPostprocessor
    expression = 'f_total - f_diff - f_adv'
    pp_names = 'flux_left_total flux_left_diff flux_left_adv'
    pp_symbols = 'f_total f_diff f_adv'
  []
  [flux_right_sum_error]
    type = ParsedPostprocessor
    expression = 'f_total - f_diff - f_adv'
    pp_names = 'flux_right_total flux_right_diff flux_right_adv'
    pp_symbols = 'f_total f_diff f_adv'
  []
  [flux_balance]
    type = ParsedPostprocessor
    expression = 'f_left + f_right'
    pp_names = 'flux_left_total flux_right_total'
    pp_symbols = 'f_left f_right'
  []
  # These are the analytic solutions, when we refine we get closer and closer
  [analytic_flux_left_adv]
    type = ParsedPostprocessor
    expression = '-v'
    constant_names = 'v'
    constant_expressions = '0.5'
  []
  [analytic_flux_left_diff]
    type = ParsedPostprocessor
    expression = '-v / (exp(v / k * L) - 1)'
    constant_names = 'v k L'
    constant_expressions = '0.5 0.5 1'
  []
  [analytic_flux_right_adv]
    type = ParsedPostprocessor
    expression = '0'
  []
  [analytic_flux_right_diff]
    type = ParsedPostprocessor
    expression = 'v * exp(v / k * L) / (exp(v / k * L) - 1)'
    constant_names = 'v k L'
    constant_expressions = '0.5 0.5 1'
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_tol = 1e-12
  l_abs_tol = 1e-12
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]
