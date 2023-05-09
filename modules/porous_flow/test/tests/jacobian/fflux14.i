# 1phase, 2components (water and salt using BrineFluidProperties), constant insitu permeability
# Constant relative perm, nonzero gravity

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    xmin = 0
    xmax = 10
    ny = 1
    ymin = 0
    ymax = 10
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 -10 0'
[]

[Variables]
  [pp]
  []
  [xnacl]
  []
[]

[ICs]
  [pp]
    type = RandomIC
    variable = pp
    min = 1e6
    max = 2e6
  []
  [massfrac0]
    type = RandomIC
    variable = xnacl
    min = 0.1
    max = 0.2
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    variable = pp
    fluid_component = 0
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    variable = xnacl
    fluid_component = 1
  []
  [flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pp
  []
  [flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = xnacl
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp xnacl'
    number_fluid_phases = 1
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
    pc = 0
  []
[]

[FluidProperties]
  [brine]
    type = BrineFluidProperties
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
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'xnacl'
  []
  [brine]
    type = PorousFlowBrine
    phase = 0
    xnacl = xnacl
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-14 0 0 0 2e-14 0 0 0 3e-14'
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [relperm]
    type = PorousFlowRelativePermeabilityConst
    kr = 1
    phase = 0
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
  dt = 1
  end_time = 1
[]

[Outputs]
  exodus = false
[]

