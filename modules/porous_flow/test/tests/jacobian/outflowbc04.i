# PorousFlowOutflowBC: testing Jacobian for multi-phase, multi-component
[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 3
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '1 2 3'
[]

[Variables]
  [pwater]
    initial_condition = 1
  []
  [pgas]
    initial_condition = 2
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pwater
  []
  [flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pwater
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = pgas
  []
  [flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = pgas
  []
[]

[AuxVariables]
  [frac_water_in_liquid]
    initial_condition = 0.6
  []
  [frac_water_in_gas]
    initial_condition = 0.4
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas pwater'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    alpha = 1
    m = 0.6
  []
[]

[FluidProperties]
  [simple_fluid0]
    type = SimpleFluidProperties
    bulk_modulus = 1.5
    density0 = 1.2
    cp = 0.9
    cv = 1.1
    viscosity = 0.4
    thermal_expansion = 0.7
  []
  [simple_fluid1]
    type = SimpleFluidProperties
    bulk_modulus = 2.5
    density0 = 0.5
    cp = 1.9
    cv = 2.1
    viscosity = 0.9
    thermal_expansion = 0.4
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = 0
  []
  [saturation_calculator]
    type = PorousFlow2PhasePP
    phase0_porepressure = pwater
    phase1_porepressure = pgas
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'frac_water_in_liquid frac_water_in_gas'
  []
  [water]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid0
    phase = 0
  []
  [co2]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid1
    phase = 1
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.2
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '0.1 0.2 0.3 1.8 0.9 1.7 0.4 0.3 1.1'
  []
  [relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
    s_res = 0.0
    sum_s_res = 0.0
  []
  [relperm_gas]
    type = PorousFlowRelativePermeabilityBC
    nw_phase = true
    lambda = 2
    s_res = 0.0
    sum_s_res = 0.0
    phase = 1
  []
[]

[BCs]
  [outflow0]
    type = PorousFlowOutflowBC
    boundary = 'front back top bottom'
    variable = pwater
    mass_fraction_component = 0
    multiplier = 1E8 # so this BC gets weighted much more heavily than Kernels
  []
  [outflow1]
    type = PorousFlowOutflowBC
    boundary = 'left right top bottom'
    variable = pgas
    mass_fraction_component = 1
    multiplier = 1E8 # so this BC gets weighted much more heavily than Kernels
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
  solve_type = NEWTON
  dt = 1E-7
  num_steps = 1
#  petsc_options = '-snes_test_jacobian -snes_force_iteration'
#  petsc_options_iname = '-snes_type --ksp_type -pc_type -snes_convergence_test'
#  petsc_options_value = ' ksponly     preonly   none     skip'
[]
