[Mesh]
  [gen_1d_edge2]
    type = GeneratedMeshGenerator
    dim = 1
    elem_type = EDGE2
    subdomain_ids = '1'
    boundary_id_offset = 0
    boundary_name_prefix = 'gen_1d_edge2'
  []
  [gen_1d_edge3]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 2
    xmax = 3
    elem_type = EDGE3
    subdomain_ids = '2'
    boundary_id_offset = 10
    boundary_name_prefix = 'gen_1d_edge3'
  []
  [gen_2d_tri3]
    type = GeneratedMeshGenerator
    dim = 2
    ymin = 2
    ymax = 3
    elem_type = TRI3
    subdomain_ids = '3'
    boundary_id_offset = 20
    boundary_name_prefix = 'gen_2d_tri3'
  []
  [gen_2d_tri6]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 2
    xmax = 3
    ymin = 2
    ymax = 3
    elem_type = TRI6
    subdomain_ids = '4'
    boundary_id_offset = 30
    boundary_name_prefix = 'gen_2d_tri6'
  []
  [gen_2d_tri7]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 4
    xmax = 5
    ymin = 2
    ymax = 3
    elem_type = TRI7
    subdomain_ids = '5'
    boundary_id_offset = 40
    boundary_name_prefix = 'gen_2d_tri7'
  []
  [gen_2d_quad4]
    type = GeneratedMeshGenerator
    dim = 2
    ymin = 4
    ymax = 5
    elem_type = QUAD4
    subdomain_ids = '6'
    boundary_id_offset = 50
    boundary_name_prefix = 'gen_2d_quad4'
  []
  [gen_2d_quad8]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 2
    xmax = 3
    ymin = 4
    ymax = 5
    elem_type = QUAD8
    subdomain_ids = '7'
    boundary_id_offset = 60
    boundary_name_prefix = 'gen_2d_quad8'
  []
  [gen_2d_quad9]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 4
    xmax = 5
    ymin = 4
    ymax = 5
    elem_type = QUAD9
    subdomain_ids = '8'
    boundary_id_offset = 70
    boundary_name_prefix = 'gen_2d_quad9'
  []
  [gen_3d_hex8]
    type = GeneratedMeshGenerator
    dim = 3
    ymin = 6
    ymax = 7
    elem_type = HEX8
    subdomain_ids = '9'
    boundary_id_offset = 80
    boundary_name_prefix = 'gen_3d_hex8'
  []
  [gen_3d_hex20]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 2
    xmax = 3
    ymin = 6
    ymax = 7
    elem_type = HEX20
    subdomain_ids = '10'
    boundary_id_offset = 90
    boundary_name_prefix = 'gen_3d_hex20'
  []
  [gen_3d_hex27]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 4
    xmax = 5
    ymin = 6
    ymax = 7
    elem_type = HEX27
    subdomain_ids = '11'
    boundary_id_offset = 100
    boundary_name_prefix = 'gen_3d_hex27'
  []
  [gen_3d_tet4]
    type = GeneratedMeshGenerator
    dim = 3
    ymin = 8
    ymax = 9
    elem_type = TET4
    subdomain_ids = '12'
    boundary_id_offset = 110
    boundary_name_prefix = 'gen_3d_tet4'
  []
  [gen_3d_tet10]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 2
    xmax = 3
    ymin = 8
    ymax = 9
    elem_type = TET10
    subdomain_ids = '13'
    boundary_id_offset = 120
    boundary_name_prefix = 'gen_3d_tet10'
  []
  [gen_3d_tet14]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 4
    xmax = 5
    ymin = 8
    ymax = 9
    elem_type = TET14
    subdomain_ids = '14'
    boundary_id_offset = 130
    boundary_name_prefix = 'gen_3d_tet14'
  []
  [gen_3d_prism6]
    type = GeneratedMeshGenerator
    dim = 3
    ymin = 10
    ymax = 11
    elem_type = PRISM6
    subdomain_ids = '15'
    boundary_id_offset = 140
    boundary_name_prefix = 'gen_3d_prism6'
  []
  [gen_3d_prism15]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 2
    xmax = 3
    ymin = 10
    ymax = 11
    elem_type = PRISM15
    subdomain_ids = '16'
    boundary_id_offset = 150
    boundary_name_prefix = 'gen_3d_prism15'
  []
  [gen_3d_prism18]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 4
    xmax = 5
    ymin = 10
    ymax = 11
    elem_type = PRISM18
    subdomain_ids = '17'
    boundary_id_offset = 160
    boundary_name_prefix = 'gen_3d_prism18'
  []
  [gen_3d_pyramid5]
    type = GeneratedMeshGenerator
    dim = 3
    ymin = 12
    ymax = 13
    elem_type = PYRAMID5
    subdomain_ids = '18'
    boundary_id_offset = 170
    boundary_name_prefix = 'gen_3d_pyramid5'
  []
  [gen_3d_pyramid13]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 2
    xmax = 3
    ymin = 12
    ymax = 13
    elem_type = PYRAMID13
    subdomain_ids = '19'
    boundary_id_offset = 180
    boundary_name_prefix = 'gen_3d_pyramid13'
  []
  [gen_3d_pyramid14]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 4
    xmax = 5
    ymin = 12
    ymax = 13
    elem_type = PYRAMID14
    subdomain_ids = '20'
    boundary_id_offset = 190
    boundary_name_prefix = 'gen_3d_pyramid14'
  []
  [cmbn]
    type = CombinerGenerator
    inputs = 'gen_1d_edge2 gen_1d_edge3
              gen_2d_tri3 gen_2d_tri6 gen_2d_tri7
              gen_2d_quad4 gen_2d_quad8 gen_2d_quad9
              gen_3d_hex8 gen_3d_hex20 gen_3d_hex27
              gen_3d_tet4 gen_3d_tet10 gen_3d_tet14
              gen_3d_prism6 gen_3d_prism15 gen_3d_prism18
              gen_3d_pyramid5 gen_3d_pyramid13 gen_3d_pyramid14'
  []
  [order_conversion]
    type = ElementOrderConversionGenerator
    input = cmbn
    conversion_type = FIRST_ORDER
  []
[]
