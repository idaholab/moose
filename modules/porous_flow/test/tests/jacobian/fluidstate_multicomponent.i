# Tests correct calculation of the property derivatives, including the spatial-gradient block of
# PorousFlowFluidState, for a three-component, two-phase fluid state. With three components there
# are two independent total mass fractions (z0, z1), which exercises the terms that sum over every
# total mass fraction: the saturation and capillary pressure gradients (through the advective
# flux) and the mass fraction gradients (through the dispersive flux). The fluid state is a test
# object whose properties are simple functions of pressure, temperature and both total mass
# fractions, so no flash calculation is involved.

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
  [pgas]
  []
  [z0]
  []
  [z1]
  []
[]

[ICs]
  [pgas]
    type = RandomIC
    min = 1e6
    max = 2e6
    variable = pgas
    seed = 1
  []
  [z0]
    type = RandomIC
    min = 0.3
    max = 0.4
    variable = z0
    seed = 2
  []
  [z1]
    type = RandomIC
    min = 0.2
    max = 0.3
    variable = z1
    seed = 3
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    variable = pgas
    fluid_component = 0
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    variable = z0
    fluid_component = 1
  []
  [mass2]
    type = PorousFlowMassTimeDerivative
    variable = z1
    fluid_component = 2
  []
  [adv0]
    type = PorousFlowAdvectiveFlux
    variable = pgas
    fluid_component = 0
  []
  [adv1]
    type = PorousFlowAdvectiveFlux
    variable = z0
    fluid_component = 1
  []
  [adv2]
    type = PorousFlowAdvectiveFlux
    variable = z1
    fluid_component = 2
  []
  [diff0]
    type = PorousFlowDispersiveFlux
    variable = pgas
    fluid_component = 0
    disp_long = '0 0'
    disp_trans = '0 0'
  []
  [diff1]
    type = PorousFlowDispersiveFlux
    variable = z0
    fluid_component = 1
    disp_long = '0 0'
    disp_trans = '0 0'
  []
  [diff2]
    type = PorousFlowDispersiveFlux
    variable = z1
    fluid_component = 2
    disp_long = '0 0'
    disp_trans = '0 0'
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas z0 z1'
    number_fluid_phases = 2
    number_fluid_components = 3
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1e-4
    pc_max = 1e5
  []
  [fs]
    type = PorousFlowMultiComponentTestFluidState
    number_fluid_components = 3
    capillary_pressure = pc
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = 300
  []
  [fluidstate]
    type = PorousFlowFluidState
    gas_porepressure = pgas
    z = 'z0 z1'
    temperature_unit = Kelvin
    capillary_pressure = pc
    fluid_state = fs
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-12 0 0 0 1e-12 0 0 0 1e-12'
  []
  [relperm0]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
  [relperm1]
    type = PorousFlowRelativePermeabilityCorey
    n = 3
    phase = 1
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [diffusivity]
    type = PorousFlowDiffusivityConst
    diffusion_coeff = '1e-2 2e-2 3e-2 4e-2 5e-2 6e-2'
    tortuosity = '0.1 0.2'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 1
  end_time = 1
  nl_abs_tol = 1e-12
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]
