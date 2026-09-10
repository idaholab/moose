# The p-multigrid preconditioner's coarse levels are function spaces on the fine mesh, built from
# the fine system's variables at reduced order. A HIERARCHIC fine space makes the level a subset of
# the fine space: level p = 1 carries the vertex modes, which are the whole of the 25-dof bilinear
# space on this mesh, out of the 1089 dofs of the order-8 space.
#
# The configuration here is the canonical 8-4-2-1 v-cycle: an order-8 fine space over coarse levels of
# order 4, 2 and 1.

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
    # HIERARCHIC shape functions above first order require a second-order geometric element
    elem_type = QUAD9
  []
[]

[Variables]
  [u]
    order = EIGHTH
    family = HIERARCHIC
  []
[]

[Kernels]
  [diff]
    type = KokkosDiffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    variable = u
    preset = false
    boundary = left
    value = 0
  []
  [right]
    type = KokkosDirichletBC
    variable = u
    preset = false
    boundary = right
    value = 1
  []
[]

[Preconditioning]
  [pmg]
    type = PMG
    level_orders = '1 2 4'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  use_kokkos_matrix_free_jacobian = true

  # Reaching the default nonlinear tolerance at order 8 takes a second Newton step whose linear solve
  # is far more expensive than the first, and one step already resolves this linear problem
  nl_rel_tol = 1e-4

  # The linear solver PMG configures, stated rather than left implicit. These are the settings PMG
  # makes through the PETSc C API, so supplying them here changes no iteration count; they are active
  # so that the configuration cannot drift out of date the way a comment describing it could.
  #
  # PCMG numbers levels from the coarsest, so for the schedule above:
  #
  #   level 0   p = 1   solved rather than smoothed, and the only level that assembles a matrix
  #   level 1   p = 2   smoothed
  #   level 2   p = 4   smoothed
  #   level 3   p = 8   the solver system itself, smoothed
  #
  # The coarsest level runs a fixed number of Krylov iterations with its convergence test disabled,
  # which is what PCMG does to its own smoothers. Stopping on a tolerance instead would make the work
  # depend on the right-hand side, and a cycle that is not a fixed linear operator is not a valid
  # preconditioner for a Krylov method that assumes one. Twenty iterations reach the same fine
  # iteration counts as a converged coarse solve and as a direct factorization of that level, and
  # '-mg_coarse_ksp_type preonly -mg_coarse_pc_type lu -mg_coarse_pc_factor_mat_solver_type mumps'
  # is a verified alternative that needs an external factorization package in parallel.
  #
  # GMRES is the outer accelerator. The operator and the cycle are both symmetric, measured, so MINRES
  # is admissible and was tried: it costs 21 iterations here against GMRES's 14, and 155 against 88
  # under 'smoother = point_jacobi'. The two are equivalent in exact arithmetic, both minimizing the
  # residual over the same space, so that difference is the short Lanczos recurrence losing
  # orthogonality on an ill-conditioned system where GMRES orthogonalizes explicitly. MINRES remains
  # the cheaper choice per iteration in storage, so the trade is a real one at larger scale.
  #
  # Two settings are deliberately absent because a parameter above already owns them, and stating them
  # twice would let the two disagree. The level count follows 'level_orders' and PETSc errors out if
  # '-pc_mg_levels' contradicts it. The preconditioner on each smoothed level follows 'smoother', and
  # '-mg_levels_pc_type' would silently replace the shell that 'smoother = entity_block' installs.
  #
  # Each level's operator and interpolation have no command-line spelling either, being shells: the
  # operator contracts the quadrature-point Jacobian cache against that level's basis, and the
  # interpolation is the element embedding between consecutive levels.
  petsc_options_iname = '-ksp_type -pc_type -pc_mg_galerkin -pc_mg_cycle_type '
                        '-mg_coarse_ksp_type -mg_coarse_ksp_norm_type -mg_coarse_ksp_max_it '
                        '-mg_coarse_pc_type '
                        '-mg_levels_ksp_type -mg_levels_ksp_max_it '
                        '-mg_levels_ksp_chebyshev_esteig -mg_levels_esteig_ksp_type'
  petsc_options_value = 'gmres mg none v '
                        'cg none 20 '
                        'gamg '
                        'chebyshev 2 '
                        '0,0.1,0,1.1 gmres'
[]

[Outputs]
  exodus = true
[]
