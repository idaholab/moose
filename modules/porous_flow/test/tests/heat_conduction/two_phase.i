# 2phase heat conduction, with saturation fixed at 0.5
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
  [phase0_porepressure]
    initial_condition = 0
  []
  [phase1_saturation]
    initial_condition = 0.5
  []
  [temp]
    initial_condition = 200
  []
[]

[Kernels]
  [dummy_p0]
    type = TimeDerivative
    variable = phase0_porepressure
  []
  [dummy_s1]
    type = TimeDerivative
    variable = phase1_saturation
  []
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
    porous_flow_vars = 'temp phase0_porepressure phase1_saturation'
    number_fluid_phases = 2
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
    pc = 0
  []
[]

[FluidProperties]
  [simple_fluid0]
    type = SimpleFluidProperties
    bulk_modulus = 1.5
    density0 = 0.4
    thermal_expansion = 0
    cv = 1
  []
  [simple_fluid1]
    type = SimpleFluidProperties
    bulk_modulus = 0.5
    density0 = 0.3
    thermal_expansion = 0
    cv = 2
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temp
  []
  [thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '0.3 0 0  0 0 0  0 0 0'
    wet_thermal_conductivity = '1.7 0 0  0 0 0  0 0 0'
    exponent = 1.0
    aqueous_phase_number = 1
  []
  [ppss]
    type = PorousFlow2PhasePS
    phase0_porepressure = phase0_porepressure
    phase1_saturation = phase1_saturation
    capillary_pressure = pc
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.8
  []
  [rock_heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 1.0
    density = 0.25
  []
  [simple_fluid0]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid0
    phase = 0
  []
  [simple_fluid1]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid1
    phase = 1
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
  file_base = two_phase
  [csv]
    type = CSV
  []
  exodus = false
[]
