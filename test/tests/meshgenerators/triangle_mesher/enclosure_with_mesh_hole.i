[MeshGenerators]
  [square]
    type = TriangleMesher
    points = '0 0 4 0 4 4 0 4'
    segment_subdivisions = '20 20 20 20'
    min_angle = 20
    # to strictly not adding new points on boundary for stitching
    allow_adding_points_on_boundary = false
  []
  [tri]
    type = TriangleMesher
    points = '-10 -10 2 -10 10 5 5 5 5 10 -10 10'
    segment_subdivisions = '40 60 40 40 50 100'
    segment_markers = '2 3 4 5 6 7'
    circular_hole_centers = '-5 -5'
    circular_hole_radii = 1
    circular_hole_num_side_points = 80
    min_angle = 20
    max_area = 0.05
    mesh_holes = square
    # to strictly not adding new points on boundary for stitching
    allow_adding_points_on_boundary = false
  []
  [stitch]
    type = StitchedMeshGenerator
    inputs = 'tri square'
    stitch_boundaries_pairs = '1 1'
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
