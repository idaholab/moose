# This test solves a 1D transient heat equation with a complicated thermal
# conductivity in order to verify jacobian calculation via AD

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
  xmax = 1
  ymax = 1
  zmax = 1
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
    axis_scaling_vector = '1e3 1e2 1e1'
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
  [./top]
    type = ADFunctionPresetBC
    variable = T
    boundary = top
    function = 't+1'
  [../]
  [./bottom]
    type = ADPresetBC
    variable = T
    boundary = bottom
    value = 2
  [../]

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

  [./front]
    type = ADPresetBC
    variable = T
    boundary = front
    value = 1.2
  [../]
  [./back]
    type = ADFunctionPresetBC
    variable = T
    boundary = back
    function = 't*1.4+1'
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
    point = '0.2 0.2 0.2'
    variable = T
  [../]
[]

[Outputs]
  csv = true
  exodus = true
[]
