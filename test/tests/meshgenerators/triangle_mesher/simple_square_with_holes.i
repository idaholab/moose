[MeshGenerators]
  [tri]
    type = TriangleMesher
    points = '0 0 1 0 1 1 0 1'
    min_angle = 20
    max_area = 0.005
    verbose = true

    circular_hole_centers = '0.5 0.5 0.8 0.5'
    circular_hole_radii = '0.15 0.1'
    circular_hole_num_side_points = '12 8'
    segment_markers = '2 3 4 5'
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
