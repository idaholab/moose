[Mesh]
  type = MooseMesh
  file = perfect.e
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
  [./left_to_right]
    type = QuadratureGapHeatTransfer
    variable = temp
    boundary = leftright
    paired_boundary = rightleft
  [../]
  [./right_to_left]
    type = QuadratureGapHeatTransfer
    variable = temp
    boundary = rightleft
    paired_boundary = leftright
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

[Executioner]
  type = Steady
  petsc_options = -snes_mf_operator
[]

[Output]
  output_initial = true
  exodus = true
[]

