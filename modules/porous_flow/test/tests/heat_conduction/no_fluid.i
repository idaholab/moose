# 0phase heat conduction.
# apply a boundary condition of T=300 to a bar that
# is initially at T=200, and observe the expected
# error-function response
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 100
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [temp]
    initial_condition = 200
  []
[]

[Kernels]
  [energy_dot]
    type = PorousFlowEnergyTimeDerivative
    variable = temp
  []
  [heat_conduction]
    type = PorousFlowHeatConduction
    variable = temp
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'temp'
    number_fluid_phases = 0
    number_fluid_components = 0
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temp
  []
  [thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '2.2 0 0  0 0 0  0 0 0'
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [rock_heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 2.2
    density = 0.5
  []
[]

[BCs]
  [left]
    type = DirichletBC
    boundary = left
    value = 300
    variable = temp
  []
[]


[Preconditioning]
  [andy]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1E1
  end_time = 1E2
[]

[Postprocessors]
  [t000]
    type = PointValue
    variable = temp
    point = '0 0 0'
    execute_on = 'initial timestep_end'
  []
  [t010]
    type = PointValue
    variable = temp
    point = '10 0 0'
    execute_on = 'initial timestep_end'
  []
  [t020]
    type = PointValue
    variable = temp
    point = '20 0 0'
    execute_on = 'initial timestep_end'
  []
  [t030]
    type = PointValue
    variable = temp
    point = '30 0 0'
    execute_on = 'initial timestep_end'
  []
  [t040]
    type = PointValue
    variable = temp
    point = '40 0 0'
    execute_on = 'initial timestep_end'
  []
  [t050]
    type = PointValue
    variable = temp
    point = '50 0 0'
    execute_on = 'initial timestep_end'
  []
  [t060]
    type = PointValue
    variable = temp
    point = '60 0 0'
    execute_on = 'initial timestep_end'
  []
  [t070]
    type = PointValue
    variable = temp
    point = '70 0 0'
    execute_on = 'initial timestep_end'
  []
  [t080]
    type = PointValue
    variable = temp
    point = '80 0 0'
    execute_on = 'initial timestep_end'
  []
  [t090]
    type = PointValue
    variable = temp
    point = '90 0 0'
    execute_on = 'initial timestep_end'
  []
  [t100]
    type = PointValue
    variable = temp
    point = '100 0 0'
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  file_base = no_fluid
  [csv]
    type = CSV
  []
  exodus = false
[]
