# PorousFlowPorosity at the nodes, with an elemental reference_temperature.
#
# A CONSTANT MONOMIAL AuxVariable holds one value per element, not one per node,
# so a nodal Material must read it at the quadpoints rather than by degree of
# freedom.  Before the fix PorousFlowPorosity always read it by degree of
# freedom and ran off the end of the one-entry array.  That read is silent in an
# optimised build, so this test is only meaningful under a devel or dbg build,
# where MooseArray's bounds assertion catches it.
#
# Note there is no mixed order and no second-order mesh anywhere here: a plain
# HEX8 with a first-order porepressure is enough.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 1
    nz = 1
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [porepressure]
    initial_condition = 1e6
  []
[]

[AuxVariables]
  [T_ref]
    family = MONOMIAL
    order = CONSTANT # one degree of freedom per element, not one per node
    initial_condition = 300
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = porepressure
  []
  [flux]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = porepressure
    gravity = '0 0 0'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = porepressure
    boundary = left
    value = 2e6
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'porepressure'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    density0 = 1000
    thermal_expansion = 0
    viscosity = 1e-3
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = 300
  []
  [eff_p]
    type = PorousFlowEffectiveFluidPressure
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = porepressure
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosity
    thermal = true
    fluid = true
    porosity_zero = 0.1
    thermal_expansion_coeff = 1e-5
    solid_bulk = 1e10
    biot_coefficient = 1
    reference_temperature = T_ref # the elemental AuxVariable
    reference_porepressure = 1e6
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-14 0 0  0 1e-14 0  0 0 1e-14'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityConst
    phase = 0
    kr = 1.0
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    solve_type = NEWTON
  []
[]

[Postprocessors]
  # PorousFlowPorosity is thermal, so the porosity depends on
  # (temperature - reference_temperature).  The fluid mass integrates the nodal
  # porosity, and so changes if reference_temperature is read incorrectly
  [fluid_mass]
    type = PorousFlowFluidMass
    fluid_component = 0
    execute_on = 'initial timestep_end'
  []
  [pp_centre]
    type = PointValue
    variable = porepressure
    point = '0.5 0.5 0.5'
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 1
[]

[Outputs]
  csv = true
[]
