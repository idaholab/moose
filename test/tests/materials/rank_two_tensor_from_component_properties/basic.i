[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 20
    ny = 20
    nz = 20
    xmax = 0.1
    ymax = 0.1
    zmax = 0.1
  []
[]

[Variables]
  [temperature]
    initial_condition = 300
  []
[]

[Kernels]
  [heat_conduction]
    type = AnisoHeatConduction
    variable = temperature
    thermal_conductivity = 'thermal_conductivity'
  []

  [heat_source]
    type = HeatSource
    variable = temperature
    value = 1000000
  []
[]

[Materials]
[k_x_property]
  type = ParsedMaterial
  property_name = k_x
  coupled_variables = 'temperature'
  expression = '1.0 +0.005*temperature '
[]

[k_y_property]
  type = ParsedMaterial
  property_name = k_y
  coupled_variables = 'temperature'
  expression = '10.0 +0.05*temperature'
[]

  # Assemble into tensor
  [thermal_conductivity_tensor]
    type = RankTwoTensorFromComponentProperties
    tensor_name = 'thermal_conductivity'
    tensor_values = 'k_x       0.0     0.0
                     0.0       k_y     0.0
                     0.0       0.0     1.0'
  []
[]

[BCs]
  [fixed]
    type = DirichletBC
    variable = temperature
    boundary = 'left right top bottom front back'
    value = 300
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [temp_max]
    type = ElementExtremeValue
    variable = temperature
    value_type = max
  []

  [temp_center]
    type = PointValue
    variable = temperature
    point = '0.05 0.05 0.05'
  []
[]
