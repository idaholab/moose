# The physics and the p-multigrid hierarchy shared by every solver configuration exercised in this
# directory. Each of pmultigrid.i and pmultigrid_petsc_options.i includes this file and adds its own
# Executioner, so that the two spell the same linear solver two different ways.
#
# The coarse levels are function spaces on the fine mesh, built from the fine system's variables at
# reduced order. A HIERARCHIC fine space makes a level a subset of the fine space: level p = 1
# carries the vertex modes, which are the whole of the 25-dof bilinear space on this mesh, out of the
# 1089 dofs of the order-8 space. The hierarchy configured here is the canonical 8-4-2-1 v-cycle.

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

    # The default smoother, pinned here because the iteration counts the tests in this directory cap
    # are reached with it: it inverts the block of degrees of freedom each mesh entity carries, which
    # the diagonal smoother cannot damp at this order
    smoother = entity_block
  []
[]

[Postprocessors]
  # The residual norm of the initial guess. Nothing about it depends on how the mesh is partitioned,
  # which makes it a check on the constrained rows being written by their owner alone: a row written
  # once by its owner and again by every process that ghosts it lands in the norm several times over,
  # and does so only at the rank counts that put a constrained degree of freedom on a partition
  # boundary.
  [initial_residual]
    type = Residual
    residual_type = INITIAL
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  exodus = true
[]
