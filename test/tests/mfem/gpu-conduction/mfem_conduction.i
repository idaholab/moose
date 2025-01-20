[Mesh]
  type = MFEMMesh
  file = 'mesh_in.e'
  dim = 3
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [H1FESpace]
    type = MFEMFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[Variables]
  [T]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

# [AuxVariables]
#   [power_density]
#     family = LAGRANGE
#     order = FIRST
#   []
# []

[ICs]
  # [power]
  #   type = ConstantIC
  #   variable = power_density
  #   value = 7675296.2525
  #   block = 'FUEL_11 FUEL_12 FUEL_13 FUEL_21 FUEL_22 FUEL_23
  #            FUEL_TRI_11 FUEL_TRI_12 FUEL_TRI_13 FUEL_TRI_21 FUEL_TRI_22 FUEL_TRI_23'
  # []
  [T_ic]
    type = MFEMScalarIC
    coefficient = T_ic_function
    variable = T
  []
[]

[Functions]
  [T_ic_function]
    type = ParsedFunction
    expression = 1000
  []
[]

[Kernels]
  [conduction]
    type = MFEMDiffusionKernel
    variable = T
    coefficient = thermal_conductivity
  []
  # [heat]
  #   type = CoupledForce
  #   variable = T
  #   v = power_density
  # []
[]

[Materials]
  [fuel]
    type = MFEMGenericConstantMaterial
    block = 'FUEL_11 FUEL_12 FUEL_13 FUEL_21 FUEL_22 FUEL_23
             FUEL_TRI_11 FUEL_TRI_12 FUEL_TRI_13 FUEL_TRI_21 FUEL_TRI_22 FUEL_TRI_23'
    prop_names = 'thermal_conductivity'
    prop_values = '1000'
  []
  [monolith]
    type = MFEMGenericConstantMaterial
    block = 'MONOLITH MONOLITH_TRI'
    prop_names = 'thermal_conductivity'
    prop_values = '1830'
  []
  [reflector]
    type = MFEMGenericConstantMaterial
    block = 'REFLECTOR REFLECTOR_TRI'
    prop_names = 'thermal_conductivity'
    prop_values = '200'
  []
  [gap]
    type = MFEMGenericConstantMaterial
    block = 'GAP'
    prop_names = 'thermal_conductivity'
    prop_values = '0.08'
  []
  [htpipe]
    type = MFEMGenericConstantMaterial
    prop_names = 'htpipe_tinf htpipe_htc'
    prop_values = '800 372'
  []
  [bnd]
    type = MFEMGenericConstantMaterial
    prop_names = 'bnd_tinf bnd_htc'
    prop_values = '300 30'
  []
[]

[BCs]
  [htpipe]
    type = MFEMConvectiveHeatFluxBC
    variable = T
    boundary = 'htpipe'
    T_infinity = htpipe_tinf
    heat_transfer_coefficient = htpipe_htc
  []
  [convection]
    type = MFEMConvectiveHeatFluxBC
    variable = T
    boundary = 'bottom top outer'
    T_infinity = bnd_tinf
    heat_transfer_coefficient = bnd_htc
  []
  # [radiation]
  #   type = RadiativeHeatFluxBC
  #   variable = T
  #   boundary = 'bottom top outer'
  #   Tinfinity = 300
  #   boundary_emissivity = 0.7
  # []
[]

[Solver]
  type = MFEMHypreGMRES
  preconditioner = boomeramg
  l_tol = 1e-8
  l_max_its = 1000
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]
