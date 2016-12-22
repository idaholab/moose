# Assign porosity and permeability variables from constant AuxVariables to create
# a heterogeneous model

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 3
  ny = 3
  nz = 3
  xmin = 1
  xmax = 4
  ymin = 1
  ymax = 4
  zmin = 1
  zmax = 4
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 -10'
[]

[Variables]
  [./ppwater]
    initial_condition = 1e6
  [../]
[]

[AuxVariables]
  [./poro]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./permxx]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./permxy]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./permxz]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./permyx]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./permyy]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./permyz]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./permzx]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./permzy]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./permzz]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./poromat]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./permxxmat]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./permxymat]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./permxzmat]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./permyxmat]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./permyymat]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./permyzmat]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./permzxmat]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./permzymat]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./permzzmat]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./poromat]
    type = MaterialRealAux
    property = PorousFlow_porosity_qp
    variable = poromat
  [../]
  [./permxxmat]
    type = MaterialRealTensorValueAux
    property = PorousFlow_permeability_qp
    variable = permxxmat
    column = 0
    row = 0
  [../]
  [./permxymat]
    type = MaterialRealTensorValueAux
    property = PorousFlow_permeability_qp
    variable = permxymat
    column = 1
    row = 0
  [../]
  [./permxzmat]
    type = MaterialRealTensorValueAux
    property = PorousFlow_permeability_qp
    variable = permxzmat
    column = 2
    row = 0
  [../]
  [./permyxmat]
    type = MaterialRealTensorValueAux
    property = PorousFlow_permeability_qp
    variable = permyxmat
    column = 0
    row = 1
  [../]
  [./permyymat]
    type = MaterialRealTensorValueAux
    property = PorousFlow_permeability_qp
    variable = permyymat
    column = 1
    row = 1
  [../]
  [./permyzmat]
    type = MaterialRealTensorValueAux
    property = PorousFlow_permeability_qp
    variable = permyzmat
    column = 2
    row = 1
  [../]
  [./permzxmat]
    type = MaterialRealTensorValueAux
    property = PorousFlow_permeability_qp
    variable = permzxmat
    column = 0
    row = 2
  [../]
  [./permzymat]
    type = MaterialRealTensorValueAux
    property = PorousFlow_permeability_qp
    variable = permzymat
    column = 1
    row = 2
  [../]
  [./permzzmat]
    type = MaterialRealTensorValueAux
    property = PorousFlow_permeability_qp
    variable = permzzmat
    column = 2
    row = 2
  [../]
[]

[ICs]
  [./poro]
    type = RandomIC
    seed = 0
    variable = poro
    max = 0.5
    min = 0.1
  [../]
  [./permxx]
    type = FunctionIC
    function = permxx
    variable = permxx
  [../]
  [./permxy]
    type = FunctionIC
    function = permxy
    variable = permxy
  [../]
  [./permxz]
    type = FunctionIC
    function = permxz
    variable = permxz
  [../]
  [./permyx]
    type = FunctionIC
    function = permyx
    variable = permyx
  [../]
  [./permyy]
    type = FunctionIC
    function = permyy
    variable = permyy
  [../]
  [./permyz]
    type = FunctionIC
    function = permyz
    variable = permyz
  [../]
  [./permzx]
    type = FunctionIC
    function = permzx
    variable = permzx
  [../]
  [./permzy]
    type = FunctionIC
    function = permzy
    variable = permzy
  [../]
  [./permzz]
    type = FunctionIC
    function = permzz
    variable = permzz
  [../]
[]

[Functions]
  [./permxx]
    type = ParsedFunction
    value = '(x*x)*1e-11'
  [../]
  [./permxy]
    type = ParsedFunction
    value = '(x*y)*1e-11'
  [../]
  [./permxz]
    type = ParsedFunction
    value = '(x*z)*1e-11'
  [../]
  [./permyx]
    type = ParsedFunction
    value = '(y*x)*1e-11'
  [../]
  [./permyy]
    type = ParsedFunction
    value = '(y*y)*1e-11'
  [../]
  [./permyz]
    type = ParsedFunction
    value = '(y*z)*1e-11'
  [../]
  [./permzx]
    type = ParsedFunction
    value = '(z*x)*1e-11'
  [../]
  [./permzy]
    type = ParsedFunction
    value = '(z*y)*1e-11'
  [../]
  [./permzz]
    type = ParsedFunction
    value = '(z*z)*1e-11'
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    variable = ppwater
  [../]
  [./flux0]
    type = PorousFlowAdvectiveFlux
    variable = ppwater
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'ppwater'
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
  [./ppss]
    type = PorousFlow1PhaseP
    porepressure = ppwater
  [../]
  [./ppss_nodal]
    type = PorousFlow1PhaseP
    at_nodes = true
    porepressure = ppwater
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1000
    bulk_modulus = 2e9
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    include_old = true
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./dens0_qp]
    type = PorousFlowDensityConstBulk
    density_P0 = 1000
    bulk_modulus = 2e9
    phase = 0
  [../]
  [./dens_qp_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
    at_nodes = false
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    at_nodes = true
    porosity = poro
  [../]
  [./porosity_qp]
    type = PorousFlowPorosityConst
    porosity = poro
  [../]
  [./visc0]
    type = PorousFlowViscosityConst
    at_nodes = true
    viscosity = 1e-3
    phase = 0
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_viscosity_nodal
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConstFromVar
    perm_xx = permxx
    perm_xy = permxy
    perm_xz = permxz
    perm_yx = permyx
    perm_yy = permyy
    perm_yz = permyz
    perm_zx = permzx
    perm_zy = permzy
    perm_zz = permzz
  [../]
  [./relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    at_nodes = true
    n = 2
    phase = 0
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_relative_permeability_nodal
  [../]
[]

[Postprocessors]
  [./mass_ph0]
    type = PorousFlowFluidMass
    fluid_component = 0
    execute_on = 'initial timestep_end'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol'
    petsc_options_value = 'bcgs bjacobi 1E-12 1E-10'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 100
  dt = 100
[]

[Outputs]
  execute_on = 'initial timestep_end'
  exodus = true
  print_perf_log = true
[]
