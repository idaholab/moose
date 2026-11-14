# Two primary nodes with unequal weights, so that each weight must stay matched to its own primary
# node. The secondary node is constrained to 0.25*u(node 0) + 0.75*u(node 6). Node 0 lies on the
# bottom boundary where u = 1 and node 6 lies on the top boundary where u = 3, so the secondary
# node is driven to 0.25*1 + 0.75*3 = 2.5. Swapping the weights, or applying either weight to the
# wrong primary node, gives 1.5 instead.
#
# A single primary node cannot catch such a mismatch, which is why this test uses two.

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  # Node ids run along x and then y for this generator, so node 0 is at (0,0), node 5 is at
  # (1,0.5) and node 6 is at (0,1). Node 5 is the only node on the right boundary that no
  # Dirichlet condition also acts on.
  [secondary]
    type = ExtraNodesetGenerator
    input = square
    new_boundary = secondary
    nodes = '5'
  []
  allow_renumbering = false
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
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  csv = true
[]
