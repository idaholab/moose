# The p-multigrid preconditioner's coarse levels are function spaces on the fine mesh, built from
# the fine system's variables at reduced order. A HIERARCHIC fine space makes the level a subset of
# the fine space: level p = 1 carries the vertex modes, which are the whole of the 25-dof bilinear
# space on this mesh, out of the 169 dofs of the cubic space.

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
    order = THIRD
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

# The linear solver PMG configures, written as the PETSc command line that would produce it. PMG makes
# these settings through the C API, so the list documents the configuration rather than driving it, and
# supplying it alongside changes no iteration count.
#
# For a fine space of order 8 over level_orders = '1 2 4' -- the canonical 8-4-2-1 v-cycle -- PCMG
# numbers levels from the coarsest:
#
#   level 0   p = 1   solved rather than smoothed, and the only level that assembles a matrix
#   level 1   p = 2   smoothed
#   level 2   p = 4   smoothed
#   level 3   p = 8   the solver system itself, smoothed
#
#   -pc_type mg
#   -pc_mg_levels 4                one more than the number of entries in level_orders
#   -pc_mg_galerkin none           every level's operator is supplied, not formed by PETSc
#   -pc_mg_cycle_type v
#   -mg_coarse_ksp_type cg         the coarsest level is iterated to convergence, not cycled once
#   -mg_coarse_ksp_rtol 1e-10
#   -mg_coarse_ksp_max_it 200
#   -mg_coarse_pc_type gamg
#   -mg_levels_ksp_type chebyshev  every smoothed level, which is levels 1 through 3
#   -mg_levels_ksp_max_it 2        a fixed two applications, which is PCMG's own default
#   -mg_levels_pc_type jacobi      what smoother = point_jacobi gives
#
# PCMG also leaves each smoother's convergence test disabled, which is what makes a smoother a fixed
# linear operator rather than one whose work depends on its right-hand side, and it estimates the
# Chebyshev eigenvalue bounds itself, equivalent to
# '-mg_levels_ksp_chebyshev_esteig 0,0.1,0,1.1' over '-mg_levels_esteig_ksp_type gmres'. The lower
# bound of that transform is a tenth of the estimated largest eigenvalue, so the bottom of the
# spectrum is deliberately left to the coarse levels rather than smoothed.
#
# Three pieces have no command-line spelling, being shells this preconditioner supplies: each level's
# operator, which contracts the quadrature-point Jacobian cache against that level's basis; each
# level's interpolation, which is the element embedding between consecutive levels; and the per-level
# preconditioner that smoother = entity_block installs in place of jacobi above.
#
# The level count follows level_orders, so the list is written for that one cycle rather than set in
# this file, where it would contradict any other choice of level_orders.

[Preconditioning]
  [pmg]
    type = PMG
    level_orders = '1'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  use_kokkos_matrix_free_jacobian = true
[]

[Outputs]
  exodus = true
[]
