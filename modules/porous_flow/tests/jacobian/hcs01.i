# apply a half-cubic sink flux and observe the correct behavior
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [./ppwater]
  [../]
  [./ppgas]
  [../]
  [./massfrac_ph0_sp0]
  [../]
  [./massfrac_ph1_sp0]
  [../]
[]


[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'ppwater ppgas massfrac_ph0_sp0 massfrac_ph1_sp0'
    number_fluid_phases = 2
    number_fluid_components = 2
  [../]
[]

[ICs]
  [./ppwater]
    type = RandomIC
    variable = ppwater
    min = -1
    max = 0
  [../]
  [./ppgas]
    type = RandomIC
    variable = ppgas
    min = 0
    max = 1
  [../]
  [./massfrac_ph0_sp0]
    type = RandomIC
    variable = massfrac_ph0_sp0
    min = 0
    max = 1
  [../]
  [./massfrac_ph1_sp0]
    type = RandomIC
    variable = massfrac_ph1_sp0
    min = 0
    max = 1
  [../]
[]

[Kernels]
  [./dummy_ppwater]
    type = TimeDerivative
    variable = ppwater
  [../]
  [./dummy_ppgas]
    type = TimeDerivative
    variable = ppgas
  [../]
  [./dummy_m00]
    type = TimeDerivative
    variable = massfrac_ph0_sp0
  [../]
  [./dummy_m10]
    type = TimeDerivative
    variable = massfrac_ph1_sp0
  [../]
[]

[Materials]
  [./temperature_nodal]
    type = PorousFlowTemperature
    at_nodes = true
  [../]
  [./ppss_nodal]
    type = PorousFlow2PhasePP_VG
    phase0_porepressure = ppwater
    phase1_porepressure = ppgas
    at_nodes = true
    al = 1
    m = 0.5
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph1_sp0'
  [../]
  [./dens0_nodal]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1
    bulk_modulus = 1.5
    phase = 0
  [../]
  [./dens1_nodal]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 0.5
    bulk_modulus = 0.5
    phase = 1
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    include_old = false
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./visc0]
    type = PorousFlowViscosityConst
    at_nodes = true
    viscosity = 1
    phase = 0
  [../]
  [./visc1]
    type = PorousFlowViscosityConst
    at_nodes = true
    viscosity = 1.4
    phase = 1
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_viscosity_nodal
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    at_nodes = false
    permeability = '1 0 0 0 2 0 0 0 3'
  [../]
  [./relperm0]
    type = PorousFlowRelativePermeabilityCorey
    at_nodes = true
    n = 2
    phase = 0
  [../]
  [./relperm1]
    type = PorousFlowRelativePermeabilityCorey
    at_nodes = true
    n = 3
    phase = 1
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_relative_permeability_nodal
  [../]
[]

[BCs]
  [./flux_w]
    type = PorousFlowHalfCubicSink
    boundary = 'left'
    center = 0.1
    cutoff = -1.1
    max = 2.2
    variable = ppwater
    mass_fraction_component = 0
    fluid_phase = 0
    use_relperm = true
    use_mobility = true
    flux_function = 'x*y'
  [../]
  [./flux_g]
    type = PorousFlowHalfCubicSink
    boundary = 'top left front'
    center = 0.5
    cutoff = -1.1
    max = -2.2
    mass_fraction_component = 0
    variable = ppgas
    fluid_phase = 1
    use_relperm = true
    use_mobility = true
    flux_function = '-x*y'
  [../]
  [./flux_1]
    type = PorousFlowHalfCubicSink
    boundary = 'right'
    center = -0.1
    cutoff = -1.1
    max = 1.2
    mass_fraction_component = 1
    variable = massfrac_ph0_sp0
    fluid_phase = 1
    use_relperm = true
    use_mobility = true
    flux_function = '-1.1*x*y'
  [../]
  [./flux_2]
    type = PorousFlowHalfCubicSink
    boundary = 'bottom'
    center = 3.2
    cutoff = -1.1
    max = 1.2
    mass_fraction_component = 1
    variable = massfrac_ph1_sp0
    fluid_phase = 1
    use_relperm = true
    use_mobility = true
    flux_function = '0.5*x*y'
  [../]
[]


[Preconditioning]
  [./check]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 2
[]

[Outputs]
  file_base = hcs01
[]
