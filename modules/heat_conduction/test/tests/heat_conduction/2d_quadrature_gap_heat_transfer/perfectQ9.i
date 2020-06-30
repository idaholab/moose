[GlobalParams]
  order = SECOND
[]

[Mesh]
  file = perfectQ9.e
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
    value = 300
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
    emissivity_primary = 0
    emissivity_secondary = 0
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
[]

[Executioner]
  type = Steady

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  [./Quadrature]
    order = THIRD
  [../]
[]

[Outputs]
  exodus = true
[]
