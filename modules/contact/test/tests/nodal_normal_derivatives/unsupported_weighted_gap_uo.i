!include ../cohesive_zone_model/mortar_czm.i

[Variables]
  [normal_lm]
    block = secondary_lower
  []
[]

[Constraints]
  [unsupported_weighted_gap]
    type = ComputeWeightedGapLMMechanicalContact
    primary_boundary = top_bottom
    secondary_boundary = bottom_top
    primary_subdomain = primary_lower
    secondary_subdomain = secondary_lower
    variable = normal_lm
    disp_x = disp_x
    disp_y = disp_y
    weighted_gap_uo = czm_uo
  []
[]
