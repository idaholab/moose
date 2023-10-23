# This test solves a 1D transient heat equation with a complicated thermal
# conductivity in order to verify jacobian calculation via AD

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  xmax = 0.001
  ymax = 0.001
[]

[Variables]
  [./T]
    initial_condition = 1.5
  [../]
  [./c]
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
  [./c]
    type = ADDiffusion
    variable = c
  [../]
[]

[Kernels]
  [./c_dt]
    type = TimeDerivative
    variable = c
  [../]
[]

[BCs]
  [./left_c]
    type = DirichletBC
    variable = c
    boundary = left
    value = 2
  [../]
  [./right_c]
    type = DirichletBC
    variable = c
    boundary = right
    value = 1
  [../]
  [./left_T]
    type = DirichletBC
    variable = T
    boundary = top
    value = 1
  [../]
  [./right_T]
    type = DirichletBC
    variable = T
    boundary = bottom
    value = 2
  [../]
[]

[Materials]
  [./k]
    type = ADThermalConductivityTest
    c = c
    temperature = T
  [../]
[]

[Preconditioning]
  [./full]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
