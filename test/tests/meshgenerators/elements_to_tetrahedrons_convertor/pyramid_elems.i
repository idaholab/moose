p0 = '0 0 0'
p1 = '1 0 0'
p2 = '1 1 0'
p3 = '0 1 0'
p4 = '0.5 0.5 1'

[Mesh]
  [pyramid5_0]
    type = ElementGenerator
    elem_type = PYRAMID5
    create_sidesets = true
    element_connectivity = '0 1 2 3 4'
    nodal_positions = '${p0} ${p1} ${p2} ${p3} ${p4}'
  []
  [pyramid5_1]
    type = ElementGenerator
    elem_type = PYRAMID5
    create_sidesets = true
    element_connectivity = '3 0 1 2 4'
    nodal_positions = '${p1} ${p2} ${p3} ${p0} ${p4}'
  []
  [pyramid5_2]
    type = ElementGenerator
    elem_type = PYRAMID5
    create_sidesets = true
    element_connectivity = '2 3 0 1 4'
    nodal_positions = '${p2} ${p3} ${p0} ${p1} ${p4}'
  []
  [pyramid5_3]
    type = ElementGenerator
    elem_type = PYRAMID5
    create_sidesets = true
    element_connectivity = '1 2 3 0 4'
    nodal_positions = '${p3} ${p0} ${p1} ${p2} ${p4}'
  []
  [cmb]
    type = CombinerGenerator
    inputs = 'pyramid5_0 pyramid5_1
              pyramid5_2 pyramid5_3'
    positions = '0 0 0 2 0 0
                 0 2 0 2 2 0'
  []
  [convert]
    type = ElementsToTetrahedronsConverter
    input = cmb
  []
[]
