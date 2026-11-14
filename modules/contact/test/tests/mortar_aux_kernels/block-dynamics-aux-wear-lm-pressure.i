# Couples the normal contact Lagrange multiplier directly as the wear pressure, which is an error
!include block-dynamics-aux-wear.i

[AuxKernels]
  inactive = 'worn_depth'
  [worn_depth_lm]
    type = MortarArchardsLawAux
    variable = worn_depth
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    displacements = 'disp_x disp_y'
    friction_coefficient = 0.5
    energy_wear_coefficient = 1.0
    normal_pressure = normal_lm
  []
[]
