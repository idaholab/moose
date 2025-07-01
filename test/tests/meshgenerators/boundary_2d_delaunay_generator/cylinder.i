[Mesh]
  [circle]
    type = ConcentricCircleMeshGenerator
    has_outer_square = false
    radii = 1
    num_sectors = 4
    rings = 1
    preserve_volumes = false
  []
  [side]
    type = SideSetsAroundSubdomainGenerator
    input = circle
    new_boundary = side
    block = 1
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = side
    heights = '2'
    num_layers = '3'
    direction = '0 0 1'
  []
  [side_1]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'x>=0&y>=0'
    new_sideset_name = 'side_1'
    input = 'extrude'
    included_boundaries = 'side'
  []
  [bd_1]
    type = Boundary2DDelaunayGenerator
    input = side_1
    boundary_names = 'side_1'
    use_auto_area_func = true
  []
  [side_2]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'x<0&y>=0'
    new_sideset_name = 'side_2'
    input = 'extrude'
    included_boundaries = 'side'
  []
  [bd_2]
    type = Boundary2DDelaunayGenerator
    input = side_2
    boundary_names = 'side_2'
    use_auto_area_func = true
  []
  [side_3]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'x<0&y<0'
    new_sideset_name = 'side_3'
    input = 'extrude'
    included_boundaries = 'side'
  []
  [bd_3]
    type = Boundary2DDelaunayGenerator
    input = side_3
    boundary_names = 'side_3'
    use_auto_area_func = true
  []
  [side_4]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'x>=0&y<0'
    new_sideset_name = 'side_4'
    input = 'extrude'
    included_boundaries = 'side'
  []
  [bd_4]
    type = Boundary2DDelaunayGenerator
    input = side_4
    boundary_names = 'side_4'
    use_auto_area_func = true
  []
  [bd_bot]
    type = Boundary2DDelaunayGenerator
    input = extrude
    boundary_names = '2'
    use_auto_area_func = true
  []
  [bd_top]
    type = Boundary2DDelaunayGenerator
    input = extrude
    boundary_names = '3'
    use_auto_area_func = true
  []
  [smg]
    type = StitchedMeshGenerator
    inputs = 'bd_1 bd_2 bd_3 bd_4 bd_top bd_bot'
    clear_stitched_boundary_ids = true
    stitch_boundaries_pairs = '0 0;0 0;0 0;0 0;0 0;0 0'
    merge_boundaries_with_same_name = true
    prevent_boundary_ids_overlap = false
  []
  [xyzd]
    type = XYZDelaunayGenerator
    boundary = smg
    desired_volume = 1
    output_subdomain_name = 'matrix'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [area]
    type = VolumePostprocessor
  []
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = 'FINAL'
  []
[]
