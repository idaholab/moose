# checking that the PorousFlowDarcyVelocityComponent AuxKernel works as expected
# for the fully-saturated case (relative-permeability = 1) using finite volumes

[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '1 -2 3'
[]

[Variables]
  [pp]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
[]

[ICs]
  [pinit]
    type = FunctionIC
    function = x
    variable = pp
  []
[]

[FVKernels]
  [mass0]
    type = FVPorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
[]

[AuxVariables]
  [vel_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [vel_y]
    order = CONSTANT
    family = MONOMIAL
  []
  [vel_z]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [vel_x]
    type = ADPorousFlowDarcyVelocityComponent
    variable = vel_x
    component = x
    fluid_phase = 0
  []
  [vel_y]
    type = ADPorousFlowDarcyVelocityComponent
    variable = vel_y
    component = y
    fluid_phase = 0
  []
  [vel_z]
    type = ADPorousFlowDarcyVelocityComponent
    variable = vel_z
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
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1e6
    viscosity = 3.2
    density0 = 1
    thermal_expansion = 0
  []
[]

[Postprocessors]
  [vel_x]
    type = PointValue
    variable = vel_x
    point = '0.5 0.5 0.5'
  []
  [vel_y]
    type = PointValue
    variable = vel_y
    point = '0.5 0.5 0.5'
  []
  [vel_z]
    type = PointValue
    variable = vel_z
    point = '0.5 0.5 0.5'
  []
[]

[Materials]
  [temperature]
    type = ADPorousFlowTemperature
  []
  [ppss]
    type = ADPorousFlow1PhaseP
    porepressure = pp
    capillary_pressure = pc
  []
  [massfrac]
    type = ADPorousFlowMassFraction
  []
  [simple_fluid]
    type = ADPorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [permeability]
    type = ADPorousFlowPermeabilityConst
    permeability = '1 0 0 0 2 0 0 0 3'
  []
  [relperm]
    type = ADPorousFlowRelativePermeabilityConst
    phase = 0
    kr = 1
  []
  [porosity]
    type = ADPorousFlowPorosityConst
    porosity = 0.1
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  nl_abs_tol = 1e-16
  dt = 1
  end_time = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = true
[]
