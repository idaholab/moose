p0 = '0 0 0'
p1 = '1 0 0'
p2 = '1 1 0'
p3 = '0 0 1'
p4 = '1 0 1'
p5 = '1 1 1'

[Mesh]
  [prism6_0]
    type = ElementGenerator
    elem_type = PRISM6
    create_sidesets = true
    element_connectivity = '0 1 2 3 4 5'
    nodal_positions = '${p0} ${p1} ${p2} ${p3} ${p4} ${p5}'
  []
  [prism6_1]
    type = ElementGenerator
    elem_type = PRISM6
    create_sidesets = true
    element_connectivity = '2 0 1 5 3 4'
    nodal_positions = '${p1} ${p2} ${p0} ${p4} ${p5} ${p3}'
  []
  [prism6_2]
    type = ElementGenerator
    elem_type = PRISM6
    create_sidesets = true
    element_connectivity = '1 2 0 4 5 3'
    nodal_positions = '${p2} ${p0} ${p1} ${p5} ${p3} ${p4}'
  []
  [prism6_3]
    type = ElementGenerator
    elem_type = PRISM6
    create_sidesets = true
    element_connectivity = '3 5 4 0 2 1'
    nodal_positions = '${p3} ${p5} ${p4} ${p0} ${p2} ${p1}'
  []
  [prism6_4]
    type = ElementGenerator
    elem_type = PRISM6
    create_sidesets = true
    element_connectivity = '4 3 5 1 0 2'
    nodal_positions = '${p4} ${p3} ${p5} ${p1} ${p0} ${p2}'
  []
  [prism6_5]
    type = ElementGenerator
    elem_type = PRISM6
    create_sidesets = true
    element_connectivity = '5 4 3 2 1 0'
    nodal_positions = '${p5} ${p4} ${p3} ${p2} ${p1} ${p0}'
  []
  [cmb]
    type = CombinerGenerator
    inputs = 'prism6_0 prism6_1 prism6_2
              prism6_3 prism6_4 prism6_5'
    positions = '0 0 0 2 0 0 4 0 0
                 0 2 0 2 2 0 4 2 0'
  []
  [convert]
    type = ElementsToTetrahedronsConverter
    input = cmb
  []
[]
