# Archard wear from a constant normal contact pressure given as a number

!include block-dynamics-aux-wear.i

[AuxVariables]
  [worn_depth_constant]
    block = '3'
  []
[]

[AuxKernels]
  [worn_depth_constant]
    type = MortarArchardsLawAux
    variable = worn_depth_constant
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    displacements = 'disp_x disp_y'
    friction_coefficient = 0.5
    energy_wear_coefficient = 1.0
    normal_pressure = 1e3
  []
[]
