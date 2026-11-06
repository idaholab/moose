# Archard wear on a partially covered (edge-dropping) interface with node-based Lagrange multiplier
# scaling. The scaling factor differs from one at the partially covered nodes, so the wear depth
# depends on whether the wear kernel reads the physical pressure or the scaled multiplier.

!include two_equal_blocks_slide_2d.i

[UserObjects/weighted_gap_uo]
  use_nodal_scaling = true
[]

[AuxVariables]
  [worn_depth]
    block = 'secondary_lower'
  []
[]

[AuxKernels]
  [worn_depth]
    type = MortarArchardsLawAux
    variable = worn_depth
    primary_boundary = '23'
    secondary_boundary = '11'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    displacements = 'disp_x disp_y'
    friction_coefficient = 0.5
    energy_wear_coefficient = 1e-6
    weighted_gap_uo = weighted_gap_uo
    correct_edge_dropping = true
    execute_on = 'TIMESTEP_END'
  []
[]

[Postprocessors]
  [max_worn_depth]
    type = NodalExtremeValue
    variable = worn_depth
    block = 'secondary_lower'
  []
  [total_worn_depth]
    type = NodalSum
    variable = worn_depth
    block = 'secondary_lower'
  []
[]

[Outputs]
  file_base = two_equal_blocks_slide_2d_wear_out
[]
