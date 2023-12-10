# 2 phase heat conduction, with saturation fixed at 0.5
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
  [phase0_porepressure]
    type = MooseVariableFVReal
    initial_condition = 0
  []
  [phase1_saturation]
    type = MooseVariableFVReal
    initial_condition = 0.5
  []
  [temp]
    type = MooseVariableFVReal
    initial_condition = 200
  []
[]

[FVKernels]
  [dummy_p0]
    type = FVTimeKernel
    variable = phase0_porepressure
  []
  [dummy_s1]
    type = FVTimeKernel
    variable = phase1_saturation
  []
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
    type = ADPorousFlowTemperature
    temperature = temp
  []
  [thermal_conductivity]
    type = ADPorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '0.3 0 0  0 0 0  0 0 0'
    wet_thermal_conductivity = '1.7 0 0  0 0 0  0 0 0'
    exponent = 1.0
    aqueous_phase_number = 1
  []
  [ppss]
    type = ADPorousFlow2PhasePS
    phase0_porepressure = phase0_porepressure
    phase1_saturation = phase1_saturation
    capillary_pressure = pc
  []
  [porosity]
    type = ADPorousFlowPorosityConst
    porosity = 0.8
  []
  [rock_heat]
    type = ADPorousFlowMatrixInternalEnergy
    specific_heat_capacity = 1.0
    density = 0.25
  []
  [simple_fluid0]
    type = ADPorousFlowSingleComponentFluid
    fp = simple_fluid0
    phase = 0
  []
  [simple_fluid1]
    type = ADPorousFlowSingleComponentFluid
    fp = simple_fluid1
    phase = 1
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
  file_base = two_phase_fv
  csv = true
[]
