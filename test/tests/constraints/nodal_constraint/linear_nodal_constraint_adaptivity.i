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
  # Node ids run along x and then y, so node 0 is at (0,0), node 5 at (1,0.5) and node 6 at (0,1).
  [secondary]
    type = ExtraNodesetGenerator
    input = square
    new_boundary = secondary
    nodes = '5'
  []
  allow_renumbering = false
[]

[Problem]
  # Refinement moves the constrained nodes' couplings, so the sparsity pattern changes between
  # time steps, as in the equal value boundary constraint adaptivity test.
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
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  automatic_scaling = true
  num_steps = 3
  nl_rel_tol = 1e-06
  nl_abs_tol = 1e-08
[]

[Outputs]
  csv = true
[]
