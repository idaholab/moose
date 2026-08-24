# A TET4 sliver sharing its base triangular face with a PRISM6 (a non-tetrahedral neighbor).
# Every collapse of the sliver's interior apex touches the prism, so the non-TET4 guard
# rejects it and the sliver is left in place (handled gracefully, no corruption).
[Mesh]
  # A PRISM6 below z=0 sharing its top triangular face (A,B,C) with a tet "tent" above.
  [prism]
    type = ElementGenerator
    nodal_positions = '0 0 -1   1 0 -1   0.5 0.866 -1   0 0 0   1 0 0   0.5 0.866 0'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [sliver]
    type = ElementGenerator
    input = prism
    nodal_positions = '0 0 0   1 0 0   0.5 0.866 0   0.5 0.289 0.01'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [s1]
    type = ElementGenerator
    input = sliver
    nodal_positions = '0 0 0   1 0 0   0.5 0.289 0.01   0.5 0.289 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [s2]
    type = ElementGenerator
    input = s1
    nodal_positions = '1 0 0   0.5 0.866 0   0.5 0.289 0.01   0.5 0.289 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [s3]
    type = ElementGenerator
    input = s2
    nodal_positions = '0.5 0.866 0   0 0 0   0.5 0.289 0.01   0.5 0.289 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [repair]
    type = MeshRepairGenerator
    input = s3
    fix_node_overlap = true
    fix_sliver_elements = true
  []
[]
[Problem]
  solve=false
[]
[Executioner]
  type=Steady
[]
