!include mortar_stats_reporter_3d_adjacent_corner_sidesets.i

[Constraints]
  [mortar_averaged]
    type = EqualValueConstraint
    primary_boundary = 'primary_right primary_top'
    secondary_boundary = 'secondary_right secondary_top'
    primary_subdomain = '11'
    secondary_subdomain = '12'
    variable = lambda
    secondary_variable = T
    mortar_3d_subpatch_plane = AVERAGED_NODAL_NORMAL
    debug_mesh = true
  []
[]
