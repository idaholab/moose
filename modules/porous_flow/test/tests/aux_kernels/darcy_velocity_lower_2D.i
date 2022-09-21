# checking that the PorousFlowDarcyVelocityComponentLowerDimensional AuxKernel works as expected in 1D+2D situation
# for the fully-saturated case (relative-permeability = 1)
# The 1_frac_in_2D_example.e has size 0.3x0.2x0, and a fracture running through its
# centre, with normal = (0, 1, 0)
# Porepressure is initialised to grad(P) = (1, 2, 0)
# Fluid_density = 2
# viscosity = 10
# relative_permeability = 1
# permeability = (5, 5, 5)  (in the bulk, measured in m^2)
# permeability = (10, 10, 10)   (in the fracture, measured in m^3)
# aperture = 0.01
# gravity = (1, 0.5, 0)
# So Darcy velocity in the bulk = (0.5, -0.5, 0)
# in the fracture grad(P) = (1, 0, 0)
# In the fracture the projected gravity vector is
# tangential_gravity = (1, 0, 0)
# So the Darcy velocity in the fracture = (100, 0, 0)

[Mesh]
  type = FileMesh
  file = 1_frac_in_2D_example.e
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '1 0.5 0'
[]

[Variables]
  [pp]
  []
[]

[ICs]
  [pinit]
    type = FunctionIC
    function = 'x+2*y'
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
    block = '2 3'
  []
  [bulk_vel_y]
    order = CONSTANT
    family = MONOMIAL
    block = '2 3'
  []
  [bulk_vel_z]
    order = CONSTANT
    family = MONOMIAL
    block = '2 3'
  []
  [fracture_vel_x]
    order = CONSTANT
    family = MONOMIAL
    block = 1
  []
  [fracture_vel_y]
    order = CONSTANT
    family = MONOMIAL
    block = 1
  []
  [fracture_vel_z]
    order = CONSTANT
    family = MONOMIAL
    block = 1
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
    aperture = 0.01
  []
  [fracture_vel_y]
    type = PorousFlowDarcyVelocityComponentLowerDimensional
    variable = fracture_vel_y
    component = y
    fluid_phase = 0
    aperture = 0.01
  []
  [fracture_vel_z]
    type = PorousFlowDarcyVelocityComponentLowerDimensional
    variable = fracture_vel_z
    component = z
    fluid_phase = 0
    aperture = 0.01
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
    type = PointValue
    variable = bulk_vel_x
    point = '0 -0.05 0'
  []
  [bulk_vel_y]
    type = PointValue
    variable = bulk_vel_y
    point = '0 -0.05 0'
  []
  [bulk_vel_z]
    type = PointValue
    variable = bulk_vel_z
    point = '0 -0.05 0'
  []
  [fracture_vel_x]
    type = PointValue
    point = '0 0 0'
    variable = fracture_vel_x
  []
  [fracture_vel_y]
    type = PointValue
    point = '0 0 0'
    variable = fracture_vel_y
  []
  [fracture_vel_z]
    type = PointValue
    point = '0 0 0'
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
    block = '2 3'
  []
  [permeability_fracture]
    type = PorousFlowPermeabilityConst
    permeability = '10 0 0 0 10 0 0 0 10'
    block = 1
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
