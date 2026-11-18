!include ../3d-mortar-contact/frictionless-mortar-3d.i

[Constraints]
  active = 'normal_x_first normal_lm_last'

  [normal_x_first]
    type = NormalMortarMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'secondary_lower'
    secondary_subdomain = 'primary_lower'
    variable = mortar_normal_lm
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = weighted_gap_uo
  []

  [normal_lm_last]
    type = ComputeWeightedGapLMMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = mortar_normal_lm
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    use_displaced_mesh = true
    weighted_gap_uo = weighted_gap_uo
    c = 1
  []
[]
