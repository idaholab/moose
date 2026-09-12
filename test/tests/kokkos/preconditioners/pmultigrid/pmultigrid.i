!include pmultigrid_base.i

# The 8-4-2-1 v-cycle on the configuration PMG builds itself, with no PETSc solver options supplied.
# This is the input that covers PMG's own defaults: a change to what setupSolver() configures shows
# up here, where pmultigrid_petsc_options.i would override it.

[Executioner]
  type = Steady
  solve_type = NEWTON
  use_kokkos_matrix_free_jacobian = true

  # The physics here is linear, so one Newton step leaves behind whatever its linear solve left. A
  # linear tolerance two decades below the default nonlinear tolerance of 1e-8 therefore converges
  # the solve in a single step. Neither end of that choice is tight: the block smoother reaches this
  # tolerance in a few iterations more than it needs for 1e-8, and the tolerance stays well above the
  # accuracy an order-8 hierarchic discretization can attain at all.
  l_tol = 1e-10
[]
