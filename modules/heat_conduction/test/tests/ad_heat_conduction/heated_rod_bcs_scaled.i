# This test solves a 1D transient heat equation with a complicated thermal
# conductivity in order to verify jacobian calculation via AD

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  xmax = 1
  ymax = 1
[]

[Problem]
  coord_type = RZ
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
    axis_scaling_vector = '1e3 1e2 0'
  [../]
  [./heat_dt]
    type = ADHeatConductionTimeDerivative
    variable = T
    specific_heat = thermal_conductivity
    density_name = thermal_conductivity
  [../]
[]

[BCs]
  [./top]
    type = FunctionDirichletBC
    variable = T
    boundary = top
    function = t
  [../]
  [./bottom]
    type = DirichletBC
    variable = T
    boundary = bottom
    value = 2
  [../]

  [./left]
    type = DirichletBC
    variable = T
    boundary = left
    value = 1
  [../]
  [./right]
    type = FunctionDirichletBC
    variable = T
    boundary = right
    function = t*2
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
[]

[Outputs]
  csv = true
[]
