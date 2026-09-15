!include pmultigrid_base.i

# The same 8-4-2-1 v-cycle as pmultigrid.i, with the linear solver stated as PETSc command-line
# options rather than left to what PMG configures. PMG applies its own settings first and reads each
# level's options afterward, so everything below is an override a user can adapt; this input exists to
# document the spelling.

[Executioner]
  type = Steady
  solve_type = NEWTON
  use_kokkos_matrix_free_jacobian = true

  l_tol = 1e-10

  # PCMG numbers levels from the coarsest, so for the hierarchy the base input requests:
  #
  #   level 0   p = 1   solved rather than smoothed, and the only level that assembles a matrix
  #   level 1   p = 2   smoothed
  #   level 2   p = 4   smoothed
  #   level 3   p = 8   the solver system itself, smoothed
  #
  # The coarsest level is factored directly, because a multigrid preconditioner has to be a fixed
  # linear operator: the outer Krylov method builds its space from repeated applications of it and
  # relates the residual its recurrence tracks to the true residual on the assumption that it does not
  # change between them. No solver package is named, which leaves the choice to PETSc: a distributed
  # factorization in parallel, and PETSc's own LU on one process.
  #
  # The four numbers of '-mg_levels_ksp_chebyshev_esteig' are the transform PETSc applies to the
  # eigenvalue estimates it measures, as 'a,b,c,d' in
  #
  #   lower bound = a * min estimate + b * max estimate
  #   upper bound = c * min estimate + d * max estimate
  #
  # so '0,0.1,0,1.1' targets the interval from a tenth of the largest estimated eigenvalue to 1.1
  # times it, and uses the smallest estimate not at all. Discarding it is deliberate on both counts: a
  # Krylov estimate of the smallest eigenvalue is inaccurate until the method has converged, and a
  # smoother should damp the top of the spectrum and leave the bottom to the coarse levels. These are
  # PETSc's own defaults for a multigrid smoother and are near optimal for this hierarchy.
  #
  # GMRES is the outer accelerator. MOOSE requests an unpreconditioned convergence norm, which PETSc
  # supports for GMRES only with right preconditioning, so this GMRES minimizes the true residual.
  # The operator and the cycle are both symmetric here, so CG and MINRES are also valid and are the
  # cheaper choices in storage, needing a fixed number of vectors where GMRES needs one per iteration.
  # They minimize a different functional once preconditioned, so compare the three in the true
  # residual rather than in what each reports.
  #
  # Two settings are deliberately absent because a parameter of the Preconditioning block already owns
  # them, and stating them twice would let the two disagree. The level count follows 'level_orders',
  # and PETSc errors out if '-pc_mg_levels' contradicts it. The preconditioner on each smoothed level
  # follows 'smoother', and '-mg_levels_pc_type' would silently replace the shell that
  # 'smoother = entity_block' installs.
  #
  # Each level's operator and interpolation have no command-line spelling either, being shells: the
  # operator contracts the quadrature-point Jacobian cache against that level's basis, and the
  # interpolation is the element embedding between consecutive levels.
  petsc_options_iname = '-ksp_type -pc_type -pc_mg_galerkin -pc_mg_cycle_type '
                        '-mg_coarse_ksp_type -mg_coarse_pc_type '
                        '-mg_levels_ksp_type -mg_levels_ksp_max_it '
                        '-mg_levels_ksp_chebyshev_esteig -mg_levels_esteig_ksp_type'
  petsc_options_value = 'gmres mg none v '
                        'preonly lu '
                        'chebyshev 2 '
                        '0,0.1,0,1.1 gmres'
[]
