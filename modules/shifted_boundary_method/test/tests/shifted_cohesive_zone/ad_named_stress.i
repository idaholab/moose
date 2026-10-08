!include rectangle_sczm_tri_material.i

[Physics/SolidMechanics/QuasiStatic/all]
  base_name = custom
[]

[Physics/SolidMechanics/ShiftedCohesiveZone/czm_ik]
  base_name = custom
  stress = bulk_stress
[]

[Materials]
  active = 'elastic_stress elasticity_tensor_in elasticity_tensor_out ad_czm stress_alias'
[]

[Materials/elastic_stress]
  base_name = custom
[]

[Materials/elasticity_tensor_in]
  base_name = custom
[]

[Materials/elasticity_tensor_out]
  base_name = custom
[]

[Materials/ad_czm]
  type = ADPureElasticTractionSeparation
  base_name = custom
  normal_stiffness = 1000
  tangent_stiffness = 1000
  boundary = 'Block1_Block2'
[]

[Materials/stress_alias]
  type = RankTwoTensorMaterialADConverter
  intra_convert = true
  ad_props_in = custom_stress
  ad_props_out = custom_bulk_stress
[]
