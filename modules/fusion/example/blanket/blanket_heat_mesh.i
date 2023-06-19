[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = meshes/FNSF-CB_OB_2.3_1.8_6.0_2.6_10.4_7.0_10.3_15_24.2_round.e
  []
[]

[Outputs]
  exodus = true
[]

[Problem]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'hypre      lu           NONZERO'
[]

[Variables]
  [temp]
    initial_condition = 500
  []
[]

[AuxVariables]
[]

[Kernels]
  [conduction]
    type = HeatConduction
    variable = temp
  []
  [ob_src]
    type = BodyForce
    variable = temp
    value = 8.675e3
#    block = 'ob_1 ob_2 ob_3 ob_4 ob_5'
  []
[]

[BCs]
  [ob_outer]
    type = DirichletBC
    variable = temp
    boundary = "FW"
    value = 500
  []
#  [ob_inner]
#    type = DirichletBC
#    variable = temp
#    boundary = "BW"
#    value = 100
#  []
[]

[Materials]
  [breeder_material]
    type = HeatConductionMaterial
    specific_heat = 1 #TODO
    thermal_conductivity = 3.0
#    density = 3400 # kg / m^3
#    block = 'ob_1 ob_2 ob_3 ob_4 ob_5'
  []
  [breeder_material_dens]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = '3400' # kg / m^3
#    block = 'ob_1 ob_2 ob_3 ob_4 ob_5'
  []
[]

[Postprocessors]
  [./volume]
    type = VolumePostprocessor
  []
[]
