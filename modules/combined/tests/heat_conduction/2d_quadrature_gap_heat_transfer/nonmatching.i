[Mesh]
  type = MooseMesh
  file = /Users/gastdr/projects/herd_trunk/elk/tests/heat_conduction/2d_quadrature_gap_heat_transfer/nonmatching.e
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

[Postprocessors]
  [./left]
    type = SideFluxIntegral
    variable = temp
    boundary = leftright
    diffusivity = thermal_conductivity
  [../]
  [./right]
    type = SideFluxIntegral
    variable = temp
    boundary = rightleft
    diffusivity = thermal_conductivity
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = -snes_mf_operator
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Output]
  output_initial = true
  exodus = true
[]

