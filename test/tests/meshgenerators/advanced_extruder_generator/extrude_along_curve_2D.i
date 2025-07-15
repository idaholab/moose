[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 3
    xmax = 1
    xmin = -1
  []
  [bscg]
    type = BSplineCurveGenerator
    start_point = '0 0 0'
    end_point = '4 2 0'
    start_direction = '0 1 0'
    end_direction = '-1 0 0'
    num_elements = 10
  []
  [ext_along_curve]
    type = AdvancedExtruderGenerator
    input = gmg
    extrusion_curve = bscg
  []
  # [stitcher]
  #   type = StitchedMeshGenerator
  #   inputs = 'gmg bscg'
  #   stitch_boundaries_pairs = '1 1'
  # []
[]