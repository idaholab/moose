p0 = '0 0 0'
p1 = '1 0 0'
p2 = '1 1 0'
p3 = '0 1 0'
p4 = '0 0 1'
p5 = '1 0 1'
p6 = '1 1 1'
p7 = '0 1 1'

[Mesh]
  [hex8_0]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '0 1 2 3 4 5 6 7'
    nodal_positions = '${p0} ${p1} ${p2} ${p3} ${p4} ${p5} ${p6} ${p7}'
  []
  [hex8_1]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '0 4 5 1 3 7 6 2'
    nodal_positions = '${p0} ${p3} ${p7} ${p4} ${p1} ${p2} ${p6} ${p5}'
  []
  [hex8_2]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '0 3 7 4 1 2 6 5'
    nodal_positions = '${p0} ${p4} ${p5} ${p1} ${p3} ${p7} ${p6} ${p2}'
  []
  [hex8_3]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '1 0 4 5 2 3 7 6'
    nodal_positions = '${p1} ${p0} ${p4} ${p5} ${p2} ${p3} ${p7} ${p6}'
  []
  [hex8_4]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '3 0 1 2 7 4 5 6'
    nodal_positions = '${p1} ${p2} ${p3} ${p0} ${p5} ${p6} ${p7} ${p4}'
  []
  [hex8_5]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '4 0 3 7 5 1 2 6'
    nodal_positions = '${p1} ${p5} ${p6} ${p2} ${p0} ${p4} ${p7} ${p3}'
  []
  [hex8_6]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '2 3 0 1 6 7 4 5'
    nodal_positions = '${p2} ${p3} ${p0} ${p1} ${p6} ${p7} ${p4} ${p5}'
  []
  [hex8_7]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '5 1 0 4 6 2 3 7'
    nodal_positions = '${p2} ${p1} ${p5} ${p6} ${p3} ${p0} ${p4} ${p7}'
  []
  [hex8_8]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '7 4 0 3 6 5 1 2'
    nodal_positions = '${p2} ${p6} ${p7} ${p3} ${p1} ${p5} ${p4} ${p0}'
  []
  [hex8_9]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '4 5 1 0 7 6 2 3'
    nodal_positions = '${p3} ${p2} ${p6} ${p7} ${p0} ${p1} ${p5} ${p4}'
  []
  [hex8_10]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '1 2 3 0 5 6 7 4'
    nodal_positions = '${p3} ${p0} ${p1} ${p2} ${p7} ${p4} ${p5} ${p6}'
  []
  [hex8_11]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '3 7 4 0 2 6 5 1'
    nodal_positions = '${p3} ${p7} ${p4} ${p0} ${p2} ${p6} ${p5} ${p1}'
  []
  [hex8_12]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '3 2 6 7 0 1 5 4'
    nodal_positions = '${p4} ${p5} ${p1} ${p0} ${p7} ${p6} ${p2} ${p3}'
  []
  [hex8_13]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '4 7 6 5 0 3 2 1'
    nodal_positions = '${p4} ${p7} ${p6} ${p5} ${p0} ${p3} ${p2} ${p1}'
  []
  [hex8_14]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '1 5 6 2 0 4 7 3'
    nodal_positions = '${p4} ${p0} ${p3} ${p7} ${p5} ${p1} ${p2} ${p6}'
  []
  [hex8_15]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '5 4 7 6 1 0 3 2'
    nodal_positions = '${p5} ${p4} ${p7} ${p6} ${p1} ${p0} ${p3} ${p2}'
  []
  [hex8_16]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '7 3 2 6 4 0 1 5'
    nodal_positions = '${p5} ${p6} ${p2} ${p1} ${p4} ${p7} ${p3} ${p0}'
  []
  [hex8_17]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '2 1 5 6 3 0 4 7'
    nodal_positions = '${p5} ${p1} ${p0} ${p4} ${p6} ${p2} ${p3} ${p7}'
  []
  [hex8_18]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '6 7 3 2 5 4 0 1'
    nodal_positions = '${p6} ${p7} ${p3} ${p2} ${p5} ${p4} ${p0} ${p1}'
  []
  [hex8_19]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '6 5 4 7 2 1 0 3'
    nodal_positions = '${p6} ${p5} ${p4} ${p7} ${p2} ${p1} ${p0} ${p3}'
  []
  [hex8_20]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '6 2 1 5 7 3 0 4'
    nodal_positions = '${p6} ${p2} ${p1} ${p5} ${p7} ${p3} ${p0} ${p4}'
  []
  [hex8_21]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '7 6 5 4 3 2 1 0'
    nodal_positions = '${p7} ${p6} ${p5} ${p4} ${p3} ${p2} ${p1} ${p0}'
  []
  [hex8_22]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '2 6 7 3 1 5 4 0'
    nodal_positions = '${p7} ${p4} ${p0} ${p3} ${p6} ${p5} ${p1} ${p2}'
  []
  [hex8_23]
    type = ElementGenerator
    elem_type = HEX8
    create_sidesets = true
    element_connectivity = '5 6 2 1 4 7 3 0'
    nodal_positions = '${p7} ${p3} ${p2} ${p6} ${p4} ${p0} ${p1} ${p5}'
  []
  [cmb]
    type = CombinerGenerator
    inputs = 'hex8_0 hex8_1 hex8_2 hex8_3 hex8_4 hex8_5 hex8_6 hex8_7
              hex8_8 hex8_9 hex8_10 hex8_11 hex8_12 hex8_13 hex8_14 hex8_15
              hex8_16 hex8_17 hex8_18 hex8_19 hex8_20 hex8_21 hex8_22 hex8_23'
    positions = '0 0 0 2 0 0 4 0 0 6 0 0 8 0 0 10 0 0 12 0 0 14 0 0
                 0 2 0 2 2 0 4 2 0 6 2 0 8 2 0 10 2 0 12 2 0 14 2 0
                 0 4 0 2 4 0 4 4 0 6 4 0 8 4 0 10 4 0 12 4 0 14 4 0'
  []
  [convert]
    type = ElementsToTetrahedronsConverter
    input = cmb
  []
[]
