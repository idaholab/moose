# 1phase, heat advecting with a moving fluid using FV

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 50
    xmin = 0
    xmax = 1
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [temp]
    type = MooseVariableFVReal
  []
  [pp]
    type = MooseVariableFVReal
  []
[]

[FVICs]
  [pp]
    type = FVFunctionIC
    variable = pp
    function = '1-x'
  []
  [temp]
    type = FVFunctionIC
    variable = temp
    function = 'if(x<0.02, 300, 200)'
  []
[]

[FVBCs]
  [pp0]
    type = FVDirichletBC
    variable = pp
    boundary = left
    value = 1
  []
  [pp1]
    type = FVDirichletBC
    variable = pp
    boundary = right
    value = 0
  []
  [hot]
    type = FVDirichletBC
    variable = temp
    boundary = left
    value = 300
  []
  [cold]
    type = FVDirichletBC
    variable = temp
    boundary = right
    value = 200
  []
[]

[FVKernels]
  [mass_dot]
    type = FVPorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [advection]
    type = FVPorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pp
  []
  [energy_dot]
    type = FVPorousFlowEnergyTimeDerivative
    variable = temp
  []
  [heat_advection]
    type = FVPorousFlowHeatAdvection
    variable = temp
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'temp pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.6
    alpha = 1.3
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 100
    density0 = 1000
    viscosity = 4.4
    thermal_expansion = 0
    cv = 2
  []
[]

[Materials]
  [temperature]
    type = ADPorousFlowTemperature
    temperature = temp
  []
  [porosity]
    type = ADPorousFlowPorosityConst
    porosity = 0.2
  []
  [rock_heat]
    type = ADPorousFlowMatrixInternalEnergy
    specific_heat_capacity = 1.0
    density = 125
  []
  [simple_fluid]
    type = ADPorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [permeability]
    type = ADPorousFlowPermeabilityConst
    permeability = '1.1 0 0 0 2 0 0 0 3'
  []
  [relperm]
    type = ADPorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
  [massfrac]
    type = ADPorousFlowMassFraction
  []
  [PS]
    type = ADPorousFlow1PhaseP
    porepressure = pp
    capillary_pressure = pc
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
  dt = 0.01
  end_time = 0.6
[]

[VectorPostprocessors]
  [T]
    type = ElementValueSampler
    sort_by = x
    variable = 'temp'
  []
[]

[Outputs]
  [csv]
    type = CSV
    execute_vector_postprocessors_on = final
  []
[]
