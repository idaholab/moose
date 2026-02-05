[Mesh]
  [inner_square_sbd_0]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 3
    xmin = -0.4
    xmax = 0.4
    ymin = -0.4
    ymax = 0.4
  []
  [inner_square]
    type = SubdomainIDGenerator
    input = inner_square_sbd_0
    subdomain_id = 1 # Exodus dislikes quad ids matching tri ids
  []
  [layer_2_bdy]
    type = PolyLineMeshGenerator
    points = '-1.0 0.0 0.0
              0.0 -1.0 0.0
              1.0 0.0 0.0
              0.0 1.0 0.0'
    loop = true
  []
  [layer_3_bdy]
    type = PolyLineMeshGenerator
    points = '-1.5 -1.5 0.0
              1.5 -1.5 0.0
              1.5 1.5 0.0
              -1.5 1.5 0.0'
    loop = true
  []
  [layer_4_bdy]
    type = PolyLineMeshGenerator
    points = '-4.0 0.0 0.0
              0.0 -4.0 0.0
              4.0 0.0 0.0
              0.0 4.0 0.0'
    loop = true
  []
  [triang_2]
    type = XYDelaunayGenerator
    boundary = 'layer_2_bdy'
    holes = 'inner_square'
    stitch_holes = 'true'
    refine_holes = 'false'
    add_nodes_per_boundary_segment = 2
    refine_boundary = false
    desired_area = 0.05
    smooth_triangulation = true
    output_subdomain_id = 2
  []
  [triang_3]
    type = XYDelaunayGenerator
    boundary = 'layer_3_bdy'
    holes = 'triang_2'
    stitch_holes = 'true'
    refine_holes = 'false'
    add_nodes_per_boundary_segment = 2
    refine_boundary = false
    desired_area = 0.1
    # smooth_triangulation = false # default
    output_subdomain_id = 3
  []
  [triang_4]
    type = XYDelaunayGenerator
    boundary = 'layer_4_bdy'
    holes = 'triang_3'
    stitch_holes = 'true'
    refine_holes = 'false'
    add_nodes_per_boundary_segment = 2
    refine_boundary = true
    desired_area = 0.2
    smooth_triangulation = true
    output_subdomain_id = 4
  []
[]

[Executioner]
  type = Steady
[]

[AuxVariables]
  [qual]
    order = CONSTANT
    family = MONOMIAL
    [AuxKernel]
      type = ElementQualityAux
      variable = 'qual'
      metric = 'ASPECT_RATIO'
    []
  []
[]

[MeshDivisions]
  [block_div]
    type = SubdomainsDivision
  []
[]

[Problem]
  solve = false
[]

[VectorPostprocessors]
  [average_q]
    type = MeshDivisionFunctorReductionVectorPostprocessor
    reduction = 'average'
    mesh_division = block_div
    functors = 'qual'
  []
[]

[Outputs]
  csv = true
[]
