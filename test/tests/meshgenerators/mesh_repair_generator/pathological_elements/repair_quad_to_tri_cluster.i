# A 3x3 grid of QUAD4 in which one interior node is pulled to within 1e-7 of its lower neighbor,
# creating a short edge shared by two quads. Collapsing that edge reduces both quads to
# triangles at once (a cluster), reshapes the two other quads incident to the collapsed node
# (movers), and leaves the surrounding quads - neighbors of the affected elements - conformal.
[Mesh]
  [q00]
    type = ElementGenerator
    nodal_positions = '0 0 0  1 0 0  1 1 0  0 1 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [q10]
    type = ElementGenerator
    input = q00
    nodal_positions = '1 0 0  2 0 0  2 1 0  1 1 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [q20]
    type = ElementGenerator
    input = q10
    nodal_positions = '2 0 0  3 0 0  3 1 0  2 1 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [q01]
    type = ElementGenerator
    input = q20
    nodal_positions = '0 1 0  1 1 0  1 2 0  0 2 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [q11]
    type = ElementGenerator
    input = q01
    nodal_positions = '1 1 0  2 1 0  2 1.0000001 0  1 2 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [q21]
    type = ElementGenerator
    input = q11
    nodal_positions = '2 1 0  3 1 0  3 2 0  2 1.0000001 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [q02]
    type = ElementGenerator
    input = q21
    nodal_positions = '0 2 0  1 2 0  1 3 0  0 3 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [q12]
    type = ElementGenerator
    input = q02
    nodal_positions = '1 2 0  2 1.0000001 0  2 3 0  1 3 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [q22]
    type = ElementGenerator
    input = q12
    nodal_positions = '2 1.0000001 0  3 2 0  3 3 0  2 3 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [repair]
    type = MeshRepairGenerator
    input = q22
    fix_node_overlap = true
    fix_elements_orientation = true
    fix_pathological_elements = true
  []
  [diagnostics]
    type = MeshDiagnosticsGenerator
    input = repair
    examine_element_volumes = ERROR
    examine_element_overlap = ERROR
    examine_non_conformality = ERROR
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
