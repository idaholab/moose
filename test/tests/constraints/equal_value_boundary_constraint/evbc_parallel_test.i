[Mesh]
  # Renumbering reorders the nodes of a distributed mesh, which makes the Exodus output incomparable
  # to the replicated one even though the solution is identical. Disable it so that a single gold
  # file covers both mesh modes and any processor count.
  allow_renumbering = false
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  []
[]

[UserObjects]
  # This relationship manager triggers remote element deletion after the constraint is added.
  [late_geometric_ghosting]
    type = TestGhostBoundarySideUserObject
    boundary = left
  []
[]

[Constraints]
  [top]
    type = EqualValueBoundaryConstraint
    variable = u
    secondary = top
    # Corner of the top boundary. Locating the primary node by coordinate keeps the constraint
    # independent of the node numbering, which differs between replicated and distributed meshes.
    primary_node_coord = '1 1 0'
    penalty = 1e7
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  # This problem is linear, so the tolerances below only control how far the linear solve is driven.
  # They are tightened well past the exodiff comparison tolerance so that the converged solution is
  # independent of the processor count and of the mesh mode.
  l_tol = 1e-10
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
