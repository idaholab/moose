# A flat sliver whose four nodes are coplanar but whose apex projects OUTSIDE the opposite face
# (off to the side). The apex-to-face flap test misses this (apex is laterally far from the
# triangle), so it is caught by the relative-VOLUME test, then collapsed. Interior apex node.
[Mesh]
  [t0]
    type = ElementGenerator
    nodal_positions = '0 0 0   1 0 0   0.5 0.866 0   0.5 -0.5 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t1]
    type = ElementGenerator
    input = t0
    nodal_positions = '0 0 0   1 0 0   0.5 0.1 1   0.5 -0.5 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t2]
    type = ElementGenerator
    input = t1
    nodal_positions = '1 0 0   0.5 0.866 0   0.5 -0.5 0   0.5 0.1 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t3]
    type = ElementGenerator
    input = t2
    nodal_positions = '0.5 0.866 0   0 0 0   0.5 -0.5 0   0.5 0.1 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t4]
    type = ElementGenerator
    input = t3
    nodal_positions = '0 0 0   1 0 0   0.5 0.3 -1   0.5 0.866 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [repair]
    type = MeshRepairGenerator
    input = t4
    fix_node_overlap = true
    fix_sliver_elements = true
  []
  [diag]
    type = MeshDiagnosticsGenerator
    input = repair
    examine_non_conformality = ERROR
    examine_element_overlap = ERROR
    examine_element_volumes = ERROR
  []
[]
[Problem]
  solve=false
[]
[Executioner]
  type=Steady
[]
