# Dispersive flux: 1 phase, 2 components, fully saturated, AD
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
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
[]

[ICs]
  [pp]
    type = RandomIC
    variable = pp
    max = 2e1
    min = 1e1
  []
  [massfrac0]
    type = RandomIC
    variable = massfrac0
    min = 0
    max = 1
  []
[]

[Kernels]
  [diff0]
    type = ADPorousFlowDispersiveFlux
    fluid_component = 0
    variable = pp
    gravity = '1 0 0'
    disp_long = 0.1
    disp_trans = 0.1
  []
  [diff1]
    type = ADPorousFlowDispersiveFlux
    fluid_component = 1
    variable = massfrac0
    gravity = '1 0 0'
    disp_long = 0.1
    disp_trans = 0.1
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp massfrac0'
    number_fluid_phases = 1
    number_fluid_components = 2
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1e7
    density0 = 10
    thermal_expansion = 0
    viscosity = 1
  []
[]

[Materials]
  [temp]
    type = ADPorousFlowTemperature
  []
  [ppss]
    type = ADPorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [massfrac]
    type = ADPorousFlowMassFraction
    mass_fraction_vars = 'massfrac0'
  []
  [simple_fluid]
    type = ADPorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [poro]
    type = ADPorousFlowPorosityConst
    porosity = 0.1
  []
  [diff]
    type = ADPorousFlowDiffusivityConst
    diffusion_coeff = '1e-2 1e-1'
    tortuosity = 1
  []
  [permeability]
    type = ADPorousFlowPermeabilityConst
    permeability = '1 0 0 0 2 0 0 0 3'
  []
  [relperm]
    type = ADPorousFlowRelativePermeabilityConst
    phase = 0
  []
[]

[Preconditioning]
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
