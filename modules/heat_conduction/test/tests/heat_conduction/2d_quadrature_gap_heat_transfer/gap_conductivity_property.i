[Mesh]
  file = perfect.e
[]

[Variables]
  [./temp]
  [../]
[]

[AuxVariables]
  [./gap_conductivity]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./hc]
    type = HeatConduction
    variable = temp
  [../]
[]

[AuxKernels]
  [./gap_conductivity]
    type = MaterialRealAux
    boundary = leftright
    property = gap_conductivity
    variable = gap_conductivity
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
    gap_conductivity = 3.0
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
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
