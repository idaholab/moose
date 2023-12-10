# 0 phase (no fluid) heat conduction using FV
# Apply a boundary condition of T=300 to a bar that
# is initially at T=200, and observe the expected
# error-function response

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmin = 0
    xmax = 100
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [temp]
    type = MooseVariableFVReal
    initial_condition = 200
  []
[]

[FVKernels]
  [energy_dot]
    type = FVPorousFlowEnergyTimeDerivative
    variable = temp
  []
  [heat_conduction]
    type = FVPorousFlowHeatConduction
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
    type = ADPorousFlowTemperature
    temperature = temp
  []
  [thermal_conductivity]
    type = ADPorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '2.2 0 0  0 0 0  0 0 0'
  []
  [porosity]
    type = ADPorousFlowPorosityConst
    porosity = 0.1
  []
  [rock_heat]
    type = ADPorousFlowMatrixInternalEnergy
    specific_heat_capacity = 2.2
    density = 0.5
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    boundary = left
    value = 300
    variable = temp
  []
[]


[Preconditioning]
  [smp]
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
  [t005]
    type = PointValue
    variable = temp
    point = '5 0 0'
    execute_on = 'initial timestep_end'
  []
  [t015]
    type = PointValue
    variable = temp
    point = '15 0 0'
    execute_on = 'initial timestep_end'
  []
  [t025]
    type = PointValue
    variable = temp
    point = '25 0 0'
    execute_on = 'initial timestep_end'
  []
  [t035]
    type = PointValue
    variable = temp
    point = '35 0 0'
    execute_on = 'initial timestep_end'
  []
  [t045]
    type = PointValue
    variable = temp
    point = '45 0 0'
    execute_on = 'initial timestep_end'
  []
  [t055]
    type = PointValue
    variable = temp
    point = '55 0 0'
    execute_on = 'initial timestep_end'
  []
  [t065]
    type = PointValue
    variable = temp
    point = '65 0 0'
    execute_on = 'initial timestep_end'
  []
  [t075]
    type = PointValue
    variable = temp
    point = '75 0 0'
    execute_on = 'initial timestep_end'
  []
  [t085]
    type = PointValue
    variable = temp
    point = '85 0 0'
    execute_on = 'initial timestep_end'
  []
  [t095]
    type = PointValue
    variable = temp
    point = '95 0 0'
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  file_base = no_fluid_fv
  csv = true
[]
