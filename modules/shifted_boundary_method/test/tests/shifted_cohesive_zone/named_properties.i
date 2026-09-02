!include rectangle_sczm_tri_material.i

[Physics/SolidMechanics/QuasiStatic/all]
  base_name = custom
[]

[Physics/SolidMechanics/ShiftedCohesiveZone/czm_ik]
  base_name = custom
  stress = bulk_stress
  tangent = bulk_tangent
  tangent_definition = stress_wrt_strain
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

[Materials/czm]
  base_name = custom
[]

[Materials/stress_alias]
  type = RankTwoTensorMaterialADConverter
  intra_convert = true
  reg_props_in = custom_stress
  reg_props_out = custom_bulk_stress
[]

[Materials/tangent_alias]
  type = RankFourTensorMaterialADConverter
  intra_convert = true
  reg_props_in = custom_Jacobian_mult
  reg_props_out = custom_bulk_tangent
[]
