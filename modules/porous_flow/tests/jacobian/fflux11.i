# 1phase, 3components, constant viscosity, constant insitu permeability
# density with constant bulk, VG relative perm with a cubic, nonzero gravity, unsaturated with VG
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
  [./pp]
  [../]
  [./massfrac0]
  [../]
  [./massfrac1]
  [../]
[]

[ICs]
  [./pp]
    type = RandomIC
    variable = pp
    min = -1.0
    max = 0.0
  [../]
  [./massfrac0]
    type = RandomIC
    variable = massfrac0
    min = 0
    max = 0.3
  [../]
  [./massfrac1]
    type = RandomIC
    variable = massfrac1
    min = 0
    max = 0.4
  [../]
[]


[Kernels]
  [./flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pp
    gravity = '-1 -0.1 0'
  [../]
  [./flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = massfrac0
    gravity = '-1 -0.1 0'
  [../]
  [./flux2]
    type = PorousFlowAdvectiveFlux
    fluid_component = 2
    variable = massfrac1
    gravity = '-1 -0.1 0'
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp massfrac0 massfrac1'
    number_fluid_phases = 1
    number_fluid_components = 3
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    at_nodes = false
  [../]
  [./temperature_nodal]
    type = PorousFlowTemperature
    at_nodes = true
  [../]
  [./ppss]
    type = PorousFlow1PhaseP_VG
    at_nodes = false
    porepressure = pp
    al = 1 # small so that most effective saturations are close to 1
    m = 0.6
  [../]
  [./ppss_nodal]
    type = PorousFlow1PhaseP_VG
    porepressure = pp
    at_nodes = true
    al = 1 # small so that most effective saturations are close to 1
    m = 0.6
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
    mass_fraction_vars = 'massfrac0 massfrac1'
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    at_nodes = false
    density_P0 = 1
    bulk_modulus = 1.5
    phase = 0
  [../]
  [./dens0_nodal]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1
    bulk_modulus = 1.5
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    include_old = true
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./dens_qp_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
    at_nodes = false
  [../]
  [./visc0]
    type = PorousFlowViscosityConst
    at_nodes = true
    viscosity = 1
    phase = 0
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_viscosity_nodal
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0 0 2 0 0 0 3'
    at_nodes = false
  [../]
  [./relperm]
    type = PorousFlowRelativePermeabilityVG
    at_nodes = true
    m = 0.6
    seff_turnover = 0.8
    phase = 0
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_relative_permeability_nodal
    at_nodes = true
  [../]
[]

[Preconditioning]
  active = check
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000'
  [../]
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
  end_time = 1
[]

[Outputs]
  exodus = false
[]
