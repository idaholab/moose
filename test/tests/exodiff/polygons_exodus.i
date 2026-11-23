[Mesh]
  # A quadrilateral and a pentagon, so that the Exodus "NSIDED" block written below holds elements
  # with differing node counts.
  [quad]
    type = ElementGenerator
    nodal_positions = '0 0 0
                       1 0 0
                       1 1 0
                       0 1 0'

    element_connectivity = '0 1 2 3'
    elem_type = "C0POLYGON"
    subdomain_name = 'polygons'
  []
  [pentagon]
    type = ElementGenerator
    input = 'quad'
    nodal_positions = '2 0 0
                       3 0 0
                       3.5 1 0
                       2.5 1.5 0
                       1.5 1 0'

    element_connectivity = '0 1 2 3 4'
    elem_type = "C0POLYGON"
    subdomain_name = 'polygons'
  []
[]
