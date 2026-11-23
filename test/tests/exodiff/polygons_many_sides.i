[Mesh]
  # Two decagons, each with a sideset per side, merged into one sideset. That sideset then
  # holds side indices up to 9 for both elements, which is what distinguishes a correct
  # (element, side) ordering from one that packs the pair into a single integer key.
  [decagon_1]
    type = ElementGenerator
    nodal_positions = '1.000000 0.000000 0
                       0.809017 0.587785 0
                       0.309017 0.951057 0
                       -0.309017 0.951057 0
                       -0.809017 0.587785 0
                       -1.000000 0.000000 0
                       -0.809017 -0.587785 0
                       -0.309017 -0.951057 0
                       0.309017 -0.951057 0
                       0.809017 -0.587785 0'

    element_connectivity = '0 1 2 3 4 5 6 7 8 9'
    elem_type = "C0POLYGON"
    subdomain_name = 'polygons'
    create_sidesets = true
  []
  [decagon_2]
    type = ElementGenerator
    input = 'decagon_1'
    nodal_positions = '5.000000 0.000000 0
                       4.809017 0.587785 0
                       4.309017 0.951057 0
                       3.690983 0.951057 0
                       3.190983 0.587785 0
                       3.000000 0.000000 0
                       3.190983 -0.587785 0
                       3.690983 -0.951057 0
                       4.309017 -0.951057 0
                       4.809017 -0.587785 0'

    element_connectivity = '0 1 2 3 4 5 6 7 8 9'
    elem_type = "C0POLYGON"
    subdomain_name = 'polygons'
    create_sidesets = true
  []
  [merge_sidesets]
    type = RenameBoundaryGenerator
    input = 'decagon_2'
    old_boundary = '0 1 2 3 4 5 6 7 8 9'
    new_boundary = '100 100 100 100 100 100 100 100 100 100'
  []
[]
