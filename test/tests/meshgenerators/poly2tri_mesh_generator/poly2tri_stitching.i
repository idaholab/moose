[Mesh]
  [./outer_bdy]
    type = PolyLineMeshGenerator
    points = '-1.0 0.0 0.0
              0.0 -1.0 0.0
              1.0 0.0 0.0
              0.0 2.0 0.0'
    loop = true
  []
  [./hole_1_sbd_0]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 3
    xmin = -0.5
    xmax = -0.3
    ymin = -0.1
    ymax = 0.1
  []
  [./hole_1]
    type = SubdomainIDGenerator
    input = hole_1_sbd_0
    subdomain_id = 1 # Exodus dislikes quad ids matching tri ids
  []
  [./hole_2_sbd_0]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 3
    xmin = 0.3
    xmax = 0.5
    ymin = -0.1
    ymax = 0.1
  []
  [./hole_2]
    type = SubdomainIDGenerator
    input = hole_2_sbd_0
    subdomain_id = 1
  []
  [./triang]
    type = Poly2TriMeshGenerator
    boundary = 'outer_bdy'
    holes = 'hole_1
             hole_2'
    stitch_holes = 'true
                    false'
    refine_holes = 'false
                    false'
    interpolate_boundary = 4
    refine_boundary = false
    desired_area = 0.05
  []

  parallel_type = replicated  # libMesh bug workaround
[]

[Outputs]
  exodus = true
[]
