# Archard wear that reads the normal contact pressure from a penalty contact user object

!include frictionless-penalty-weighted-gap.i

[AuxVariables]
  [worn_depth]
    block = 3
  []
[]

[AuxKernels]
  [worn_depth]
    type = MortarArchardsLawAux
    variable = worn_depth
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    displacements = 'disp_x disp_y'
    friction_coefficient = 0.5
    energy_wear_coefficient = 1.0
    weighted_gap_uo = weighted_gap_uo
    execute_on = 'TIMESTEP_END'
  []
[]

[Postprocessors]
  [max_worn_depth]
    type = NodalExtremeValue
    variable = worn_depth
    block = 3
  []
  [total_worn_depth]
    type = NodalSum
    variable = worn_depth
    block = 3
  []
[]

[Outputs]
  csv = true
[]
