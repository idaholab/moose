# 1phase, 3components, constant viscosity, constant insitu permeability
# density with constant bulk, BW relative perm, nonzero gravity, unsaturated with BW
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  xmin = 0
  xmax = 1
  ny = 1
  ymin = 0
  ymax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
  []
  [massfrac0]
  []
  [massfrac1]
  []
[]

[ICs]
  [pp]
    type = FunctionIC
    variable = pp
    function = -0.7+x+y
  []
  [massfrac0]
    type = RandomIC
    variable = massfrac0
    min = 0
    max = 0.3
  []
  [massfrac1]
    type = RandomIC
    variable = massfrac1
    min = 0
    max = 0.4
  []
[]

[Kernels]
  [flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pp
    gravity = '-1 -0.1 0'
  []
  [flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = massfrac0
    gravity = '-1 -0.1 0'
  []
  [flux2]
    type = PorousFlowAdvectiveFlux
    fluid_component = 2
    variable = massfrac1
    gravity = '-1 -0.1 0'
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp massfrac0 massfrac1'
    number_fluid_phases = 1
    number_fluid_components = 3
  []
  [pc]
    type = PorousFlowCapillaryPressureBW
    Sn = 0.05
    Ss = 0.9
    las = 2.2
    C = 1.5
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1.5
    density0 = 1
    thermal_expansion = 0
    viscosity = 1
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
    mass_fraction_vars = 'massfrac0 massfrac1'
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0 0 2 0 0 0 3'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityBW
    Sn = 0.05
    Ss = 0.9
    Kn = 0.02
    Ks = 0.95
    C = 1.5
    phase = 0
  []
[]

[Preconditioning]
  active = check
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000'
  []
  [check]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
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
