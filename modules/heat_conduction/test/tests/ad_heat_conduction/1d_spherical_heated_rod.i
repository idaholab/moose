# This test solves a 1D transient heat equation with a complicated thermal
# conductivity in order to verify jacobian calculation via AD

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmax = 0.001
[]

[Problem]
  coord_type = RSPHERICAL
[]

[Variables]
  [./T]
    initial_condition = 1.5
  [../]
[]

[Kernels]
  [./HeatDiff]
    type = ADHeatConduction
    variable = T
    thermal_conductivity = thermal_conductivity
  [../]
  [./heat_dt]
    type = ADHeatConductionTimeDerivative
    variable = T
    specific_heat = thermal_conductivity
    density_name = thermal_conductivity
  [../]
  [./source]
    type = ADMatHeatSource
    variable = T
    scalar = 1000
  [../]
[]

[BCs]
  [./left]
    type = ADPresetBC
    variable = T
    boundary = left
    value = 1
  [../]
  [./right]
    type = ADFunctionPresetBC
    variable = T
    boundary = right
    function = 't*2+1'
  [../]
[]

[Materials]
  [./k]
    type = ADThermalConductivityTest
    c = T
    temperature = T
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 2
  dt = 1
  automatic_scaling = true
[]

[Postprocessors]
  [./avg]
    type = ElementAverageValue
    variable = T
  [../]
  [./max]
    type = ElementExtremeValue
    variable = T
  [../]
  [./min]
    type = ElementExtremeValue
    variable = T
    value_type = min
  [../]
  [./point]
    type = PointValue
    point = '0.0002 0 0'
    variable = T
  [../]
[]

[Outputs]
  csv = true
  exodus = true
[]
