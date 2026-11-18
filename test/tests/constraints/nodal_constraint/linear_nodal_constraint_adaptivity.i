# Refinement replaces the elements the constraint had ghosted and repartitions the mesh, so the
# constraint has to rebuild its node lists and re-ghost the primary elements on every mesh change.
# The whole domain is refined, including the elements of both primary nodes and of the secondary
# node. The secondary node is held at 0.25*u(node 0) + 0.75*u(node 6) = 0.25*1 + 0.75*3 = 2.5
# throughout.

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  [secondary]
    type = ExtraNodesetGenerator
    input = square
    new_boundary = secondary
    coord = '1 0.5'
  []
  # LinearNodalConstraint takes primary nodes by ID, so they have to keep the numbering this
  # generator gives them: node 0 is at (0,0) and node 6 is at (0,1).
  allow_renumbering = false
[]

[Problem]
  # Mortar and node-face constraints declare a COUPLING relationship manager, which libMesh keeps
  # in its coupling functors and consults every time it builds the sparsity pattern. Nodal
  # constraints have none: their coupling reaches the pattern only through
  # NonlinearSystemBase::augmentSparsity(), and the entries it adds for the refined mesh do not
  # end up in the preallocated Jacobian, so assembling the constraint needs a new nonzero.
  # Hash table assembly does not preallocate, and is how the equal value boundary constraint
  # adaptivity test handles the same problem.
  use_hash_table_matrix_assembly = true
[]

[Adaptivity]
  marker = 'box'
  [Markers]
    [box]
      type = BoxMarker
      bottom_left = '0 0 0'
      top_right = '1 1 0'
      inside = 'refine'
      outside = 'do_nothing'
    []
  []
[]

[Variables]
  [u]
    family = LAGRANGE
    order = FIRST
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 1
  []
  [top]
    type = DirichletBC
    variable = u
    boundary = top
    value = 3
  []
[]

[Constraints]
  [c1]
    type = LinearNodalConstraint
    variable = u
    primary = '0 6'
    weights = '0.25 0.75'
    secondary_node_set = secondary
    penalty = 1e8
  []
[]

[Postprocessors]
  [secondary_value]
    type = PointValue
    variable = u
    point = '1 0.5 0'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  # The penalty makes the unpreconditioned residual a poor convergence measure, so judge
  # convergence on the preconditioned norm instead.
  petsc_options_iname = '-pc_type -ksp_norm_type'
  petsc_options_value = 'lu preconditioned'
  num_steps = 3
  nl_rel_tol = 1e-06
  nl_abs_tol = 1e-08
[]

[Outputs]
  csv = true
[]
