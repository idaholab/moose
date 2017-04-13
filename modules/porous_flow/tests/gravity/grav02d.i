# Checking that gravity head is established in the transient situation when 0<=saturation<=1 (note the less-than-or-equal-to).
# 2phase (PP), 2components, vanGenuchten, constant fluid bulk-moduli for each phase, constant viscosity, constant permeability, Corey relative perm.
# A boundary condition enforces porepressures at the right boundary
# For better agreement with the analytical solution (ana_pp), just increase nx

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = -1
  xmax = 0
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [./ppwater]
    initial_condition = 0
  [../]
  [./ppgas]
    initial_condition = 0.5
  [../]
[]

[AuxVariables]
  [./massfrac_ph0_sp0]
    initial_condition = 1
  [../]
  [./massfrac_ph1_sp0]
    initial_condition = 0
  [../]
[]

[BCs]
  [./ppwater]
    type = PresetBC
    boundary = right
    variable = ppwater
    value = 0
  [../]
  [./ppgas]
    type = PresetBC
    boundary = right
    variable = ppgas
    value = 0.5
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = ppwater
  [../]
  [./flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = ppwater
    gravity = '-1 0 0'
  [../]
  [./mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = ppgas
  [../]
  [./flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = ppgas
    gravity = '-1 0 0'
  [../]
[]


[Functions]
  [./ana_ppwater]
    type = ParsedFunction
    vars = 'g B p0 rho0'
    vals = '1 2 pp_water_top 1'
    value = '-B*log(exp(-p0/B)+g*rho0*x/B)' # expected pp at base
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'ppwater ppgas'
    number_fluid_phases = 2
    number_fluid_components = 2
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
  [../]
  [./temperature_nodal]
    type = PorousFlowTemperature
    at_nodes = true
  [../]
  [./ppss]
    type = PorousFlow2PhasePP_VG
    at_nodes = true
    phase0_porepressure = ppwater
    phase1_porepressure = ppgas
    m = 0.5
    al = 1
  [../]
  [./ppss_qp]
    type = PorousFlow2PhasePP_VG
    phase0_porepressure = ppwater
    phase1_porepressure = ppgas
    m = 0.5
    al = 1
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph1_sp0'
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1
    bulk_modulus = 1.2
    phase = 0
  [../]
  [./dens1]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 0.1
    bulk_modulus = 1
    phase = 1
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    include_old = true
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./dens0_qp]
    type = PorousFlowDensityConstBulk
    density_P0 = 1
    bulk_modulus = 1.2
    phase = 0
  [../]
  [./dens1_qp]
    type = PorousFlowDensityConstBulk
    density_P0 = 0.1
    bulk_modulus = 1
    phase = 1
  [../]
  [./dens_qp_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
    at_nodes = false
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    at_nodes = true
    porosity = 0.1
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
    viscosity = 0.5
    phase = 1
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_viscosity_nodal
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0  0 2 0  0 0 3'
  [../]
  [./relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    n = 1
    phase = 0
  [../]
  [./relperm_gas]
    type = PorousFlowRelativePermeabilityCorey
    n = 1
    phase = 1
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_relative_permeability_qp
  [../]
  [./relperm_water_nodal]
    type = PorousFlowRelativePermeabilityCorey
    at_nodes = true
    n = 1
    phase = 0
  [../]
  [./relperm_gas_nodal]
    type = PorousFlowRelativePermeabilityCorey
    at_nodes = true
    n = 1
    phase = 1
  [../]
  [./relperm_all_nodal]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_relative_permeability_nodal
  [../]
[]

[Postprocessors]
  [./pp_water_top]
    type = PointValue
    variable = ppwater
    point = '0 0 0'
  [../]
  [./pp_water_base]
    type = PointValue
    variable = ppwater
    point = '-1 0 0'
  [../]
  [./pp_water_analytical]
    type = FunctionValuePostprocessor
    function = ana_ppwater
    point = '-1 0 0'
  [../]
  [./ppwater_00]
    type = PointValue
    variable = ppwater
    point = '0 0 0'
  [../]
  [./ppwater_01]
    type = PointValue
    variable = ppwater
    point = '-0.1 0 0'
  [../]
  [./ppwater_02]
    type = PointValue
    variable = ppwater
    point = '-0.2 0 0'
  [../]
  [./ppwater_03]
    type = PointValue
    variable = ppwater
    point = '-0.3 0 0'
  [../]
  [./ppwater_04]
    type = PointValue
    variable = ppwater
    point = '-0.4 0 0'
  [../]
  [./ppwater_05]
    type = PointValue
    variable = ppwater
    point = '-0.5 0 0'
  [../]
  [./ppwater_06]
    type = PointValue
    variable = ppwater
    point = '-0.6 0 0'
  [../]
  [./ppwater_07]
    type = PointValue
    variable = ppwater
    point = '-0.7 0 0'
  [../]
  [./ppwater_08]
    type = PointValue
    variable = ppwater
    point = '-0.8 0 0'
  [../]
  [./ppwater_09]
    type = PointValue
    variable = ppwater
    point = '-0.9 0 0'
  [../]
  [./ppwater_10]
    type = PointValue
    variable = ppwater
    point = '-1 0 0'
  [../]
  [./ppgas_00]
    type = PointValue
    variable = ppgas
    point = '0 0 0'
  [../]
  [./ppgas_01]
    type = PointValue
    variable = ppgas
    point = '-0.1 0 0'
  [../]
  [./ppgas_02]
    type = PointValue
    variable = ppgas
    point = '-0.2 0 0'
  [../]
  [./ppgas_03]
    type = PointValue
    variable = ppgas
    point = '-0.3 0 0'
  [../]
  [./ppgas_04]
    type = PointValue
    variable = ppgas
    point = '-0.4 0 0'
  [../]
  [./ppgas_05]
    type = PointValue
    variable = ppgas
    point = '-0.5 0 0'
  [../]
  [./ppgas_06]
    type = PointValue
    variable = ppgas
    point = '-0.6 0 0'
  [../]
  [./ppgas_07]
    type = PointValue
    variable = ppgas
    point = '-0.7 0 0'
  [../]
  [./ppgas_08]
    type = PointValue
    variable = ppgas
    point = '-0.8 0 0'
  [../]
  [./ppgas_09]
    type = PointValue
    variable = ppgas
    point = '-0.9 0 0'
  [../]
  [./ppgas_10]
    type = PointValue
    variable = ppgas
    point = '-1 0 0'
  [../]
[]

[Preconditioning]
  active = andy
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-12 1E-10 10000'
  [../]
  [./check]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-12 1E-10 10000 test'
  [../]
[]


[Executioner]
  type = Transient
  solve_type = Newton
  [./TimeStepper]
    type = FunctionDT
    time_t = '1E-3 1E-2 1E-1 2E-1'
    time_dt = '1E-3 1E-2 0.2E-1 1E-1'
  [../]
  end_time = 1.0
[]

[Outputs]
  execute_on = 'initial final'
  file_base = grav02d
  [./csv]
    type = CSV
  [../]
  exodus = true
[]
