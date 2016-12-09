# 1phase
# vanGenuchten, constant-bulk density, constant porosity, 3components
# unsaturated
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [./pp]
  [../]
  [./mass_frac_comp0]
  [../]
  [./mass_frac_comp1]
  [../]
[]

[ICs]
  [./pp]
    type = RandomIC
    variable = pp
    min = -1
    max = 0
  [../]
  [./mass_frac_comp0]
    type = RandomIC
    variable = mass_frac_comp0
    min = 0
    max = 0.3
  [../]
  [./mass_frac_comp1]
    type = RandomIC
    variable = mass_frac_comp1
    min = 0
    max = 0.3
  [../]
[]


[Kernels]
  [./mass_comp0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  [../]
  [./masscomp1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = mass_frac_comp0
  [../]
  [./masscomp2]
    type = PorousFlowMassTimeDerivative
    fluid_component = 2
    variable = mass_frac_comp1
  [../]
[]


[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp mass_frac_comp0 mass_frac_comp1'
    number_fluid_phases = 1
    number_fluid_components = 3
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    at_nodes = true
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
    mass_fraction_vars = 'mass_frac_comp0 mass_frac_comp1'
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1
    bulk_modulus = 1.5
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    at_nodes = true
    include_old = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    at_nodes = true
    porosity = 0.1
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
    petsc_options = '-snes_test_display'
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
  exodus = false
[]
