# The p-multigrid preconditioner's coarse levels are function spaces on the fine mesh, built from
# the fine system's variables at reduced order. A HIERARCHIC fine space makes the level a subset of
# the fine space: level p = 1 carries the vertex modes, which are the whole of the 25-dof bilinear
# space on this mesh, out of the 1089 dofs of the order-8 space.
#
# The configuration here is the canonical 8-4-2-1 v-cycle: an order-8 fine space over coarse levels of
# order 4, 2 and 1, smoothed by the entity-block Schwarz smoother.

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

    # The default smoother, pinned here because the iteration counts this input's comments record are
    # measured with it: it inverts the block of degrees of freedom each mesh entity carries, converging
    # the order-eight solve in 14 linear iterations where the diagonal smoother needs 88
    smoother = entity_block
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  use_kokkos_matrix_free_jacobian = true

  # The physics here is linear, so one Newton step leaves behind whatever its linear solve left. A
  # linear tolerance two decades below the default nonlinear tolerance of 1e-8 therefore converges the
  # solve in a single step, and leaves margin for a linear problem whose linear solve stops nearer its
  # own tolerance than this one does. Neither end of that choice is tight: the block smoother reaches
  # this tolerance in a few iterations more than it needs for 1e-8, and the tolerance stays well above
  # the accuracy an order-8 hierarchic discretization can attain at all.
  l_tol = 1e-10

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
  # The coarsest level is factored directly, by MUMPS so that the same configuration serves in
  # parallel. A multigrid preconditioner has to be a fixed linear operator, since the outer Krylov
  # method builds its space from repeated applications of it and relates its recurrence residual to the
  # true residual on the assumption that it does not change between them. A direct factorization is
  # that exactly, and it is cheap here because the coarsest level is order one on the fine mesh.
  #
  # Iterating this level instead does not qualify, on either of the two ways of stopping. A relative
  # tolerance makes the work depend on the right-hand side. A fixed iteration count fixes the work but
  # not the operator, because a Krylov method builds its polynomial from the Krylov space of the vector
  # it is handed, which makes it a nonlinear function of that vector however many steps it takes.
  #
  # GMRES is the outer accelerator. The operator and the cycle are both symmetric, measured, so MINRES
  # is admissible and was tried: it costs 21 iterations here against GMRES's 14, judged by the true
  # residual both reduce. That 1.5x is the price of a short Lanczos recurrence on a discretization whose
  # own condition number exceeds what a double represents; with the preconditioner removed, where the two
  # methods provably minimize the same quantity, the same penalty appears and tracks that condition
  # number. MINRES remains the cheaper choice per iteration in storage, needing a fixed number of vectors
  # where GMRES needs one per iteration, so the trade is a real one at larger scale.
  #
  # The two minimize different functionals once preconditioned, so compare them only in the true
  # residual. MOOSE requests an unpreconditioned convergence norm, which PETSc supports for GMRES only
  # with right preconditioning, so this GMRES minimizes exactly that true residual, while PETSc's MINRES
  # is left-preconditioned and minimizes the residual in the norm the preconditioner induces. Levelling
  # that with '-ksp_norm_type preconditioned' does not work: left-preconditioned GMRES then reduces its
  # own norm by eight orders in 12 iterations while the true relative residual stalls at 1.7e-2.
  #
  # The restart matters more than the choice between them. PETSc's default is a restart of 30 with
  # classical Gram-Schmidt and no iterative refinement, and on this discretization an unrestarted GMRES
  # loses orthogonality badly: under 'smoother = point_jacobi' it needs 503 iterations where the
  # restarted default needs 88 and where '-ksp_gmres_cgs_refinement_type refine_always' needs 42. The
  # order-8 entity-block configuration here converges in 14, so it never reaches a restart.
  #
  # The four numbers of '-mg_levels_ksp_chebyshev_esteig' are the transform PETSc applies to the
  # eigenvalue estimates it measures, as 'a,b,c,d' in
  #
  #   lower bound = a * min estimate + b * max estimate
  #   upper bound = c * min estimate + d * max estimate
  #
  # so '0,0.1,0,1.1' targets the interval from a tenth of the largest estimated eigenvalue to 1.1 times
  # it, and uses the smallest estimate not at all. Discarding it is deliberate on both counts: a Krylov
  # estimate of the smallest eigenvalue is inaccurate until the method has converged, and a smoother
  # should damp the top of the spectrum and leave the bottom to the coarse grid. The estimates come from
  # ten GMRES iterations against the level operator, and on this discretization the fine level measures a
  # largest eigenvalue near 2.8 and so targets roughly 0.28 to 3.0. These are PETSc's own defaults for a
  # multigrid smoother, and they are near optimal here: widening the upper multiplier to 2, 5 or 20 costs
  # 17, 21 and 27 linear iterations against 14, and lengthening the estimator to 20, 40 or 80 steps
  # leaves the count at 14.
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
                        '-mg_coarse_ksp_type -mg_coarse_pc_type '
                        '-mg_coarse_pc_factor_mat_solver_type '
                        '-mg_levels_ksp_type -mg_levels_ksp_max_it '
                        '-mg_levels_ksp_chebyshev_esteig -mg_levels_esteig_ksp_type'
  petsc_options_value = 'gmres mg none v '
                        'preonly lu '
                        'mumps '
                        'chebyshev 2 '
                        '0,0.1,0,1.1 gmres'
[]

[Outputs]
  exodus = true
[]
