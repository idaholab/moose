# Exception testing for PorousFlowDarcyVelocityComponentLowerDimensional
# Checking that an error is produced if the AuxVariable is not defined only on
# lower-dimensional elements
[Mesh]
  type = FileMesh
  file = fractured_block.e
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '1 0.5 0.2'
[]

[Variables]
  [pp]
  []
[]

[Kernels]
  [dummy]
    type = TimeDerivative
    variable = pp
  []
[]

[AuxVariables]
  [fracture_vel_x]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [fracture_vel_x]
    type = PorousFlowDarcyVelocityComponentLowerDimensional
    variable = fracture_vel_x
    component = x
    fluid_phase = 0
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
    pc = 0
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1E16
    viscosity = 10
    density0 = 2
    thermal_expansion = 0
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseP
    porepressure = pp
    capillary_pressure = pc
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '5 0 0 0 5 0 0 0 5'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 1
[]
