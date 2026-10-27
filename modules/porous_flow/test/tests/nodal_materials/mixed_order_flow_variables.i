# A FIRST-order porepressure alongside a SECOND-order temperature, both of them
# PorousFlow variables and so both read at the nodes.
#
# Nodal Materials index their properties with a single node counter, which has to
# be a valid degree-of-freedom index for every variable read at the nodes at once.
# Here it cannot be: the porepressure has 8 degrees of freedom on this HEX27 and
# the temperature has 27.  The Dictator therefore rejects the combination.
#
# Note that this restriction applies only to the variables read at the nodes.
# The displacements are exempt, because no Material reads them at the nodes --
# see mixed_order_mechanics.i, where SECOND-order displacements sit alongside a
# FIRST-order porepressure quite happily.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
  []
  second_order = true
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [porepressure]
    order = FIRST
    initial_condition = 1e6
  []
  [temperature]
    order = SECOND
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
  [energy]
    type = PorousFlowEnergyTimeDerivative
    variable = temperature
  []
  [heat_conduction]
    type = PorousFlowHeatConduction
    variable = temperature
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'porepressure temperature'
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
    temperature = temperature
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
  [internal_energy]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 2
    density = 2
  []
  [thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '1 0 0  0 1 0  0 0 1'
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
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

[Executioner]
  type = Transient
  dt = 1
  end_time = 1
[]

[Outputs]
  exodus = false
[]
