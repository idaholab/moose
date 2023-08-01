#Fluid Properties:
rho = 2000
vel = 1
mu = 1
#Geometric Parameters
Dh = 1
PoD = 1.10

[GlobalParams]
  execute_on = 'initial'
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  allow_renumbering = false
[]

[Materials]
  [props]
    type = ADGenericConstantMaterial
    prop_names = 'rho vel mu D_h'
    prop_values = '${rho} ${vel} ${mu} ${Dh}'
  []
  [turb]
    type = ADGenericConstantMaterial
    prop_names = 'rho_turb'
    prop_values = '2200'
  []
  [warnings]
    type = ADWallFrictionChengMaterial
    f_D = "warnings"
    PoD = ${PoD}
    bundle_array = SQUARE
    subchannel_type = INTERIOR
  []
  [PoD_105_interior_sqr_lam]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_105_interior_sqr_lam"
    PoD = 1.05
    bundle_array = SQUARE
    subchannel_type = INTERIOR
  []
  [PoD_105_edge_sqr_lam]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_105_edge_sqr_lam"
    PoD = 1.05
    bundle_array = SQUARE
    subchannel_type = EDGE
  []
  [PoD_105_corner_sqr_lam]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_105_corner_sqr_lam"
    PoD = 1.05
    bundle_array = SQUARE
    subchannel_type = CORNER
  []
  [PoD_110_interior_sqr_lam]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_110_interior_sqr_lam"
    PoD = 1.10
    bundle_array = SQUARE
    subchannel_type = INTERIOR
  []
  [PoD_110_edge_sqr_lam]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_110_edge_sqr_lam"
    PoD = 1.10
    bundle_array = SQUARE
    subchannel_type = EDGE
  []
  [PoD_110_corner_sqr_lam]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_110_corner_sqr_lam"
    PoD = 1.10
    bundle_array = SQUARE
    subchannel_type = CORNER
  []
  [PoD_105_interior_sqr_turb]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_105_interior_sqr_turb"
    PoD = 1.05
    bundle_array = SQUARE
    subchannel_type = INTERIOR
    rho = rho_turb
  []
  [PoD_105_edge_sqr_turb]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_105_edge_sqr_turb"
    PoD = 1.05
    bundle_array = SQUARE
    subchannel_type = EDGE
    rho = rho_turb
  []
  [PoD_105_corner_sqr_turb]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_105_corner_sqr_turb"
    PoD = 1.05
    bundle_array = SQUARE
    subchannel_type = CORNER
    rho = rho_turb
  []
  [PoD_110_interior_sqr_turb]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_110_interior_sqr_turb"
    PoD = 1.10
    bundle_array = SQUARE
    subchannel_type = INTERIOR
    rho = rho_turb
  []
  [PoD_110_edge_sqr_turb]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_110_edge_sqr_turb"
    PoD = 1.10
    bundle_array = SQUARE
    subchannel_type = EDGE
    rho = rho_turb
  []
  [PoD_110_corner_sqr_turb]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_110_corner_sqr_turb"
    PoD = 1.10
    bundle_array = SQUARE
    subchannel_type = CORNER
    rho = rho_turb
  []
  [PoD_105_interior_hex_lam]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_105_interior_hex_lam"
    PoD = 1.05
    bundle_array = HEXAGONAL
    subchannel_type = INTERIOR
  []
  [PoD_105_edge_hex_lam]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_105_edge_hex_lam"
    PoD = 1.05
    bundle_array = HEXAGONAL
    subchannel_type = EDGE
  []
  [PoD_105_corner_hex_lam]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_105_corner_hex_lam"
    PoD = 1.05
    bundle_array = HEXAGONAL
    subchannel_type = CORNER
  []
  [PoD_110_interior_hex_lam]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_110_interior_hex_lam"
    PoD = 1.10
    bundle_array = HEXAGONAL
    subchannel_type = INTERIOR
  []
  [PoD_110_edge_hex_lam]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_110_edge_hex_lam"
    PoD = 1.10
    bundle_array = HEXAGONAL
    subchannel_type = EDGE
  []
  [PoD_110_corner_hex_lam]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_110_corner_hex_lam"
    PoD = 1.10
    bundle_array = HEXAGONAL
    subchannel_type = CORNER
  []
  [PoD_105_interior_hex_turb]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_105_interior_hex_turb"
    PoD = 1.05
    bundle_array = HEXAGONAL
    subchannel_type = INTERIOR
    rho = rho_turb
  []
  [PoD_105_edge_hex_turb]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_105_edge_hex_turb"
    PoD = 1.05
    bundle_array = HEXAGONAL
    subchannel_type = EDGE
    rho = rho_turb
  []
  [PoD_105_corner_hex_turb]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_105_corner_hex_turb"
    PoD = 1.05
    bundle_array = HEXAGONAL
    subchannel_type = CORNER
    rho = rho_turb
  []
  [PoD_110_interior_hex_turb]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_110_interior_hex_turb"
    PoD = 1.10
    bundle_array = HEXAGONAL
    subchannel_type = INTERIOR
    rho = rho_turb
  []
  [PoD_110_edge_hex_turb]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_110_edge_hex_turb"
    PoD = 1.10
    bundle_array = HEXAGONAL
    subchannel_type = EDGE
    rho = rho_turb
  []
  [PoD_110_corner_hex_turb]
    type = ADWallFrictionChengMaterial
    f_D = "PoD_110_corner_hex_turb"
    PoD = 1.10
    bundle_array = HEXAGONAL
    subchannel_type = CORNER
    rho = rho_turb
  []
[]

[Postprocessors]
  [PoD_105_interior_sqr_lam]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_105_interior_sqr_lam
  []
  [PoD_105_edge_sqr_lam]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_105_edge_sqr_lam
  []
  [PoD_105_corner_sqr_lam]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_105_corner_sqr_lam
  []
  [PoD_110_interior_sqr_lam]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_110_interior_sqr_lam
  []
  [PoD_110_edge_sqr_lam]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_110_edge_sqr_lam
  []
  [PoD_110_corner_sqr_lam]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_110_corner_sqr_lam
  []
  [PoD_105_interior_sqr_turb]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_105_interior_sqr_turb
  []
  [PoD_105_edge_sqr_turb]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_105_edge_sqr_turb
  []
  [PoD_105_corner_sqr_turb]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_105_corner_sqr_turb
  []
  [PoD_110_interior_sqr_turb]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_110_interior_sqr_turb
  []
  [PoD_110_edge_sqr_turb]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_110_edge_sqr_turb
  []
  [PoD_110_corner_sqr_turb]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_110_corner_sqr_turb
  []
  [PoD_105_interior_hex_lam]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_105_interior_hex_lam
  []
  [PoD_105_edge_hex_lam]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_105_edge_hex_lam
  []
  [PoD_105_corner_hex_lam]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_105_corner_hex_lam
  []
  [PoD_110_interior_hex_lam]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_110_interior_hex_lam
  []
  [PoD_110_edge_hex_lam]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_110_edge_hex_lam
  []
  [PoD_110_corner_hex_lam]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_110_corner_hex_lam
  []
  [PoD_105_interior_hex_turb]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_105_interior_hex_turb
  []
  [PoD_105_edge_hex_turb]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_105_edge_hex_turb
  []
  [PoD_105_corner_hex_turb]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_105_corner_hex_turb
  []
  [PoD_110_interior_hex_turb]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_110_interior_hex_turb
  []
  [PoD_110_edge_hex_turb]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_110_edge_hex_turb
  []
  [PoD_110_corner_hex_turb]
    type = ADElementAverageMaterialProperty
    mat_prop = PoD_110_corner_hex_turb
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
