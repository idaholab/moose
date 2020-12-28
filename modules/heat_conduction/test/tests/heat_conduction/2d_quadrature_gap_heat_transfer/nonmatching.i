[Mesh]
  file = nonmatching.e
[]

[Variables]
  [./temp]
  [../]
[]

[Kernels]
  [./hc]
    type = HeatConduction
    variable = temp
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = temp
    boundary = leftleft
    value = 1000
  [../]
  [./right]
    type = DirichletBC
    variable = temp
    boundary = rightright
    value = 400
  [../]
[]

[ThermalContact]
  [./left_to_right]
    emissivity_primary = 0
    emissivity_secondary = 0
    secondary = leftright
    quadrature = true
    primary = rightleft
    variable = temp
    type = GapHeatTransfer
  [../]
[]

[Materials]
  [./hcm]
    type = HeatConductionMaterial
    block = 'left right'
    specific_heat = 1
    thermal_conductivity = 1
  [../]
  [./gap_conductance]
    type = GenericConstantMaterial
    prop_names = 'gap_conductance gap_conductance_dT'
    boundary = 'leftright rightleft'
    prop_values = '1 0'
  [../]
[]

[Postprocessors]
  [./left]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = leftright
    diffusivity = thermal_conductivity
  [../]
  [./right]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = rightleft
    diffusivity = thermal_conductivity
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
