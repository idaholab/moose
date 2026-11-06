!include rectangle_sczm_tri_material.i

[Physics/SolidMechanics/ShiftedCohesiveZone/czm_ik]
  tangent = pk1_jacobian
[]

[Materials/tangent_alias]
  type = RankFourTensorMaterialADConverter
  intra_convert = true
  reg_props_in = Jacobian_mult
  reg_props_out = pk1_jacobian
[]
