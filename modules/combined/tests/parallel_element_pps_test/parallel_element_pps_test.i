[Mesh]
  file = block_map.e
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'heat ie'

  [./heat]
    type = HeatConduction
    variable = u
  [../]

  [./ie]
    type = HeatConductionImplicitEuler
    variable = u
  [../]
[]

[BCs]
  active = 'bottom top'

  [./bottom]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0.0
  [../]

  [./top]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1.0
  [../]
[]

[Postprocessors]
   active = 'p_1 p_2 p_3 p_all'
 
  [./p_1]
    type = ElementIntegral
    variable = u
    block = '1'
  [../]

  [./p_2]
    type = ElementIntegral
    variable = u
    block = '2'
  [../]

  [./p_3]
    type = ElementIntegral
    variable = u
    block = '3'
  [../]

  [./p_all]
    type = ElementIntegral
    variable = u
    block = '1 2 3'
  [../]
[]


[Materials]
  [./constant]
    type = GenericConstantMaterial
    block = 1
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '1.0 1.0 1.0'
  [../]

  [./constant2]
    type = GenericConstantMaterial
    block = 2
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '0.8 0.8 0.8'
  [../]

  [./constant3]	
    type = GenericConstantMaterial
    block = 3
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '5 5 5'
  [../]
[]


[Executioner]
  type = Transient
  petsc_options = '-snes_mf_operator'

  start_time = 0.0
  num_steps = 5
  dt = .1
[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]

