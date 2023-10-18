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
    secondary = leftright
    quadrature = true
    primary = rightleft
    variable = temp
    emissivity_primary = 0
    emissivity_secondary = 0
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

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
