[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 5
  ymax = 5
[]

[Variables]
  [./T]
    initial_condition = 293.0 #in K
  [../]
  [./elec]
  [../]
[]

[Kernels]
  [./HeatDiff]
    type = ADHeatConduction
    variable = T
  [../]
  [./HeatTdot]
    type = ADHeatConductionTimeDerivative
    variable = T
  [../]
  [./HeatSrc]
    type = ADJouleHeatingSource
    variable = T
    elec = elec
  [../]
  [./electric]
    type = ADHeatConduction
    variable = elec
    thermal_conductivity = electrical_conductivity
  [../]
[]

[BCs]
  [./lefttemp]
    type = ADDirichletBC
    boundary = left
    variable = T
    value = 293 #in K
  [../]
  [./elec_left]
    type = ADDirichletBC
    variable = elec
    boundary = left
    value = 1 #in V
  [../]
  [./elec_right]
    type = ADDirichletBC
    variable = elec
    boundary = right
    value = 0
  [../]
[]

[Materials]
  [./k]
    type = ADGenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '397.48' #copper in W/(m K)
  [../]
  [./cp]
    type = ADGenericConstantMaterial
    prop_names = 'specific_heat'
    prop_values = '385.0' #copper in J/(kg K)
  [../]
  [./rho]
    type = ADGenericConstantMaterial
    prop_names = 'density'
    prop_values = '8920.0' #copper in kg/(m^3)
  [../]
  [./sigma] #copper is default material
    type = ADElectricalConductivity
    temperature = T
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'hypre'
  dt = 1
  end_time = 5
  automatic_scaling = true
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
