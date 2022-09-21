# checking that the PorousFlowDarcyVelocityComponentLowerDimensional AuxKernel works as expected
# for the fully-saturated case (relative-permeability = 1)
# The fractured_block.e has size = 10x10x10, and a fracture running through its
# centre, with normal = (0, -sin(20deg), cos(20deg))
# Porepressure is initialised to grad(P) = (0, 0, 1)
# Fluid_density = 2
# viscosity = 10
# relative_permeability = 1
# permeability = (5, 5, 5)  (in the bulk)
# permeability = (10, 10, 10)   (in the fracture)
# aperture = 1
# gravity = (1, 0.5, 0.2)
# So Darcy velocity in the bulk = (1, 0.5, -0.3)
# in the fracture grad(P) = (0, 0.3213938, 0.11697778)
# In the fracture the projected gravity vector is
# tangential_gravity = (1, 0.5057899, 0.18409245)
# So the Darcy velocity in the fracture = (2, 0.690186, 0.251207)

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

[ICs]
  [pinit]
    type = FunctionIC
    function = z
    variable = pp
  []
[]

[Kernels]
  [dummy]
    type = TimeDerivative
    variable = pp
  []
[]

[AuxVariables]
  [bulk_vel_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [bulk_vel_y]
    order = CONSTANT
    family = MONOMIAL
  []
  [bulk_vel_z]
    order = CONSTANT
    family = MONOMIAL
  []
  [fracture_vel_x]
    order = CONSTANT
    family = MONOMIAL
    block = 3
  []
  [fracture_vel_y]
    order = CONSTANT
    family = MONOMIAL
    block = 3
  []
  [fracture_vel_z]
    order = CONSTANT
    family = MONOMIAL
    block = 3
  []
[]

[AuxKernels]
  [bulk_vel_x]
    type = PorousFlowDarcyVelocityComponent
    variable = bulk_vel_x
    component = x
    fluid_phase = 0
  []
  [bulk_vel_y]
    type = PorousFlowDarcyVelocityComponent
    variable = bulk_vel_y
    component = y
    fluid_phase = 0
  []
  [bulk_vel_z]
    type = PorousFlowDarcyVelocityComponent
    variable = bulk_vel_z
    component = z
    fluid_phase = 0
  []
  [fracture_vel_x]
    type = PorousFlowDarcyVelocityComponentLowerDimensional
    variable = fracture_vel_x
    component = x
    fluid_phase = 0
  []
  [fracture_vel_y]
    type = PorousFlowDarcyVelocityComponentLowerDimensional
    variable = fracture_vel_y
    component = y
    fluid_phase = 0
  []
  [fracture_vel_z]
    type = PorousFlowDarcyVelocityComponentLowerDimensional
    variable = fracture_vel_z
    component = z
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

[Postprocessors]
  [bulk_vel_x]
    type = ElementAverageValue
    block = 1
    variable = bulk_vel_x
  []
  [bulk_vel_y]
    type = ElementAverageValue
    block = 1
    variable = bulk_vel_y
  []
  [bulk_vel_z]
    type = ElementAverageValue
    block = 1
    variable = bulk_vel_z
  []
  [fracture_vel_x]
    type = ElementAverageValue
    block = 3
    variable = fracture_vel_x
  []
  [fracture_vel_y]
    type = ElementAverageValue
    block = 3
    variable = fracture_vel_y
  []
  [fracture_vel_z]
    type = ElementAverageValue
    block = 3
    variable = fracture_vel_z
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
    block = '1 2'
  []
  [permeability_fracture]
    type = PorousFlowPermeabilityConst
    permeability = '10 0 0 0 10 0 0 0 10'
    block = 3
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

[Outputs]
  csv = true
[]
