# Tests correct calculation of properties derivatives in PorousFlowFluidStateWaterNCG.
# This test is run three times, with the initial condition of z (the total mass
# fraction of NCG in all phases) varied to give either a single phase liquid, a
# single phase gas, or two phases.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [./pgas]
  [../]
  [./z]
  [../]
[]

[ICs]
  active = 'pgas z_twophase'
  [./pgas]
    type = RandomIC
    min = 1e5
    max = 2e5
    variable = pgas
  [../]
  [./z_twophase]
    type = RandomIC
    min = 0.2
    max = 0.8
    variable = z
  [../]
  [./z_liquid]
    type = RandomIC
    min = 0.0001
    max = 0.0005
    variable = z
  [../]
  [./z_gas]
    type = RandomIC
    min = 0.97
    max = 0.99
    variable = z
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    variable = pgas
    fluid_component = 0
  [../]
  [./mass1]
    type = PorousFlowMassTimeDerivative
    variable = z
    fluid_component = 1
  [../]
  [./adv0]
    type = PorousFlowAdvectiveFlux
    variable = pgas
    fluid_component = 0
  [../]
  [./adv1]
    type = PorousFlowAdvectiveFlux
    variable = z
    fluid_component = 1
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas z'
    number_fluid_phases = 2
    number_fluid_components = 2
  [../]
[]

[Modules]
  [./FluidProperties]
    [./co2]
      type = CO2FluidProperties
    [../]
    [./water]
      type = Water97FluidProperties
    [../]
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    temperature = 50
  [../]
  [./temperature_nodal]
    type = PorousFlowTemperature
    temperature = 50
    at_nodes = true
  [../]
  [./waterncg]
    type = PorousFlowFluidStateWaterNCG
    gas_porepressure = pgas
    z = z
    gas_fp = co2
    water_fp = water
    at_nodes = true
    temperature_unit = Celsius
  [../]
  [./waterncg_qp]
    type = PorousFlowFluidStateWaterNCG
    gas_porepressure = pgas
    z = z
    gas_fp = co2
    water_fp = water
    temperature_unit = Celsius
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-12 0 0 0 1e-12 0 0 0 1e-12'
  [../]
  [./relperm0]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
    at_nodes = true
  [../]
  [./relperm1]
    type = PorousFlowRelativePermeabilityCorey
    n = 3
    phase = 1
    at_nodes = true
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_relative_permeability_nodal
    at_nodes = true
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
    at_nodes = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 1
  end_time = 1
  nl_abs_tol = 1e-12
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]
