# Checking that gravity head is established
# 1phase, vanGenuchten, constant and large fluid-bulk, constant viscosity, constant permeability, Corey relperm
# fully saturated
# For better agreement with the analytical solution (ana_pp), just increase nx

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmin = -1
  xmax = 0
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [./pp]
    [./InitialCondition]
      type = RandomIC
      min = 0
      max = 1
    [../]
  [../]
[]

[Kernels]
  [./flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pp
    gravity = '-1 0 0'
  [../]
[]

[Functions]
  [./ana_pp]
    type = ParsedFunction
    vars = 'g B p0 rho0'
    vals = '1 1E3 0 1'
    value = '-B*log(exp(-p0/B)+g*rho0*x/B)' # expected pp at base
  [../]
[]

[BCs]
  [./z]
    type = PresetBC
    variable = pp
    boundary = right
    value = 0
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
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
  [./ppss_qp]
    type = PorousFlow1PhaseP_VG
    porepressure = pp
    al = 1
    m = 0.5
  [../]
  [./ppss]
    type = PorousFlow1PhaseP_VG
    at_nodes = true
    porepressure = pp
    al = 1
    m = 0.5
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1
    bulk_modulus = 1E3
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    at_nodes = true
    include_old = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./dens0_qp]
    type = PorousFlowDensityConstBulk
    density_P0 = 1
    bulk_modulus = 1E3
    phase = 0
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
    permeability = '1 0 0  0 2 0  0 0 3'
  [../]
  [./relperm]
    type = PorousFlowRelativePermeabilityCorey
    at_nodes = true
    n = 1
    phase = 0
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_relative_permeability_nodal
  [../]
[]

[Postprocessors]
  [./pp_base]
    type = PointValue
    variable = pp
    point = '-1 0 0'
  [../]
  [./pp_analytical]
    type = FunctionValuePostprocessor
    function = ana_pp
    point = '-1 0 0'
  [../]
[]

[Preconditioning]
  active = andy
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
  type = Steady
  solve_type = Newton
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = grav01b
  [./csv]
    type = CSV
  [../]
[]
