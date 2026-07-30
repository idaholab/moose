!include frictionless-mortar-3d-action.i

[UserObjects]
  [mapping_conflict_oracle]
    type = LMWeightedGapUserObject
    primary_boundary = bottom_top
    secondary_boundary = top_bottom
    primary_subdomain = mortar_primary_subdomain
    secondary_subdomain = mortar_secondary_subdomain
    lm_variable = mortar_normal_lm
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    mortar_3d_qp_mapping = normal_projection
  []
[]
