# 2D diffusion MMS test with mixed Dirichlet (left/right) and Neumann (top/bottom) BCs.
# Exact solution:  u = (1.5 - x^2)(1.5 - y^2)
# Diffusion coeff: D = 1 + 0.5*x*y
# Neumann flux on bottom (n = -y): -D * du/dy|_{y=0} = 0  (du/dy = 0 at y=0)
# Neumann flux on top   (n = +y):  D * du/dy|_{y=ymax}    (outward normal flux)

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    ymax = 0.5
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
    initial_condition = 1.0
  []
[]

[LinearFVKernels]
  [diffusion]
    type = KokkosLinearFVDiffusion
    variable = u
    diffusion_coeff = coeff_func_kokkos
  []
  [source]
    type = KokkosLinearFVSource
    variable = u
    source_density = source_func_kokkos
    scaling_factor = unit_fn
  []
[]

[LinearFVBCs]
  [dirichlet_lr]
    type = KokkosLinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = 'left right'
    diffusion_coeff = coeff_func_kokkos
    functor = analytic_solution_kokkos
  []
  [neumann_bottom]
    type = KokkosLinearFVAdvectionDiffusionFunctorNeumannBC
    variable = u
    boundary = 'bottom'
    functor = neumann_bottom_flux
  []
  [neumann_top]
    type = KokkosLinearFVAdvectionDiffusionFunctorNeumannBC
    variable = u
    boundary = 'top'
    functor = neumann_top_flux
  []
[]

[Functions]
  [unit_fn]
    type = KokkosParsedFunction
    expression = 1.0
  []
  [coeff_func_kokkos]
    type = KokkosParsedFunction
    expression = '1.0 + 0.5*x*y'
  []
  [source_func_kokkos]
    type = KokkosParsedFunction
    expression = '2*(1.5-y*y) + 2*x*y*(1.5-y*y) + 2*(1.5-x*x) + 2*x*y*(1.5-x*x)'
  []
  [analytic_solution_kokkos]
    type = KokkosParsedFunction
    expression = '(1.5-x*x)*(1.5-y*y)'
  []
  [analytic_solution]
    type = ParsedFunction
    expression = '(1.5-x*x)*(1.5-y*y)'
  []
  # Outward normal flux on bottom (n = -y): flux = -D * du/dy|_{y=0}
  # du/dy = (1.5-x^2)*(-2y), at y=0: du/dy = 0, so flux = 0
  [neumann_bottom_flux]
    type = KokkosParsedFunction
    expression = 0.0
  []
  # Outward normal flux on top (n = +y): flux = D * du/dy|_{y=ymax}
  # D = 1+0.5*x*ymax, du/dy = (1.5-x^2)*(-2*ymax)
  # flux = (1+0.5*x*ymax)*(1.5-x^2)*(-2*ymax)
  [neumann_top_flux]
    type = KokkosParsedFunction
    expression = '(1.0 + 0.5*x*0.5)*(1.5-x*x)*(-2.0*0.5)'
  []
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    execute_on = FINAL
  []
  [error]
    type = ElementL2FunctorError
    approximate = u
    exact = analytic_solution
    execute_on = FINAL
  []
[]

[Convergence]
  [linear]
    type = IterationCountConvergence
    max_iterations = 1
    converge_at_max_iterations = true
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_tol = 1e-10
  multi_system_fixed_point = true
  multi_system_fixed_point_convergence = linear
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
  [console]
    type = Console
    execute_postprocessors_on = 'FINAL'
  []
[]
