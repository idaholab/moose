[Mesh]
  [gmg]
    type = CartesianMeshGenerator
    dim = 2
    ix = '1 2 2 1'
    dx = '1 1 1 1'
    # channel has details in x direction, y-discretization helps guide the tube. This was
    # not necessary for the cylindrical channel in 3D where the bends did not influence as much
    # the direction of the polyline
    iy = '5 4 4 2'
    dy = '1 1 1 1'
    # bottom is first line
    subdomain_id = '0 0 1 0
                    0 1 1 0
                    0 1 0 0
                    0 1 1 0'
  []
  [add_inner_bdy]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'gmg'
    new_boundary = 'channel'
    primary_block = '1'
    paired_block = '0'
  []
  [make_nodeset]
    type = NodeSetsFromSideSetsGenerator
    input = 'add_inner_bdy'
    output = true
  []
  [follow_channel]
    type = PolyLineMeshFollowingNodeSetGenerator
    input = 'make_nodeset'
    nodeset = 'channel'
    line_subdomain = '1d_tube'

    # start from the top, going down
    starting_point = '2 4 0'
    starting_direction = '0 -1 0'
    ignore_nodes_behind = true

    # need to hit the channel wall
    search_radius = '${fparse 1.5}'
    # Too small and we won't advance, as barycenter won't move
    # Be careful we should avoid turning back
    dx = 0.1
    max_edges = 40
    num_edges_between_points = 2
    verbose = true
  []
[]
