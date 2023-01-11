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
  [ppwater]
    initial_condition = 1e6
  []
[]

[AuxVariables]
  [poro]
    family = MONOMIAL
    order = CONSTANT
  []
  [permxx]
    family = MONOMIAL
    order = CONSTANT
  []
  [permxy]
    family = MONOMIAL
    order = CONSTANT
  []
  [permxz]
    family = MONOMIAL
    order = CONSTANT
  []
  [permyx]
    family = MONOMIAL
    order = CONSTANT
  []
  [permyy]
    family = MONOMIAL
    order = CONSTANT
  []
  [permyz]
    family = MONOMIAL
    order = CONSTANT
  []
  [permzx]
    family = MONOMIAL
    order = CONSTANT
  []
  [permzy]
    family = MONOMIAL
    order = CONSTANT
  []
  [permzz]
    family = MONOMIAL
    order = CONSTANT
  []
  [poromat]
    family = MONOMIAL
    order = CONSTANT
  []
  [permxxmat]
    family = MONOMIAL
    order = CONSTANT
  []
  [permxymat]
    family = MONOMIAL
    order = CONSTANT
  []
  [permxzmat]
    family = MONOMIAL
    order = CONSTANT
  []
  [permyxmat]
    family = MONOMIAL
    order = CONSTANT
  []
  [permyymat]
    family = MONOMIAL
    order = CONSTANT
  []
  [permyzmat]
    family = MONOMIAL
    order = CONSTANT
  []
  [permzxmat]
    family = MONOMIAL
    order = CONSTANT
  []
  [permzymat]
    family = MONOMIAL
    order = CONSTANT
  []
  [permzzmat]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [poromat]
    type = PorousFlowPropertyAux
    property = porosity
    variable = poromat
  []
  [permxxmat]
    type = PorousFlowPropertyAux
    property = permeability
    variable = permxxmat
    column = 0
    row = 0
  []
  [permxymat]
    type = PorousFlowPropertyAux
    property = permeability
    variable = permxymat
    column = 1
    row = 0
  []
  [permxzmat]
    type = PorousFlowPropertyAux
    property = permeability
    variable = permxzmat
    column = 2
    row = 0
  []
  [permyxmat]
    type = PorousFlowPropertyAux
    property = permeability
    variable = permyxmat
    column = 0
    row = 1
  []
  [permyymat]
    type = PorousFlowPropertyAux
    property = permeability
    variable = permyymat
    column = 1
    row = 1
  []
  [permyzmat]
    type = PorousFlowPropertyAux
    property = permeability
    variable = permyzmat
    column = 2
    row = 1
  []
  [permzxmat]
    type = PorousFlowPropertyAux
    property = permeability
    variable = permzxmat
    column = 0
    row = 2
  []
  [permzymat]
    type = PorousFlowPropertyAux
    property = permeability
    variable = permzymat
    column = 1
    row = 2
  []
  [permzzmat]
    type = PorousFlowPropertyAux
    property = permeability
    variable = permzzmat
    column = 2
    row = 2
  []
[]

[ICs]
  [poro]
    type = RandomIC
    seed = 0
    variable = poro
    max = 0.5
    min = 0.1
  []
  [permxx]
    type = FunctionIC
    function = permxx
    variable = permxx
  []
  [permxy]
    type = FunctionIC
    function = permxy
    variable = permxy
  []
  [permxz]
    type = FunctionIC
    function = permxz
    variable = permxz
  []
  [permyx]
    type = FunctionIC
    function = permyx
    variable = permyx
  []
  [permyy]
    type = FunctionIC
    function = permyy
    variable = permyy
  []
  [permyz]
    type = FunctionIC
    function = permyz
    variable = permyz
  []
  [permzx]
    type = FunctionIC
    function = permzx
    variable = permzx
  []
  [permzy]
    type = FunctionIC
    function = permzy
    variable = permzy
  []
  [permzz]
    type = FunctionIC
    function = permzz
    variable = permzz
  []
[]

[Functions]
  [permxx]
    type = ParsedFunction
    expression = '(x*x)*1e-11'
  []
  [permxy]
    type = ParsedFunction
    expression = '(x*y)*1e-11'
  []
  [permxz]
    type = ParsedFunction
    expression = '(x*z)*1e-11'
  []
  [permyx]
    type = ParsedFunction
    expression = '(y*x)*1e-11'
  []
  [permyy]
    type = ParsedFunction
    expression = '(y*y)*1e-11'
  []
  [permyz]
    type = ParsedFunction
    expression = '(y*z)*1e-11'
  []
  [permzx]
    type = ParsedFunction
    expression = '(z*x)*1e-11'
  []
  [permzy]
    type = ParsedFunction
    expression = '(z*y)*1e-11'
  []
  [permzz]
    type = ParsedFunction
    expression = '(z*z)*1e-11'
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    variable = ppwater
  []
  [flux0]
    type = PorousFlowAdvectiveFlux
    variable = ppwater
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'ppwater'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    density0 = 1000
    viscosity = 1e-3
    thermal_expansion = 0
    cv = 2
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = ppwater
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = poro
  []
  [permeability]
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
  []
  [relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
[]

[Postprocessors]
  [mass_ph0]
    type = PorousFlowFluidMass
    fluid_component = 0
    execute_on = 'initial timestep_end'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol'
    petsc_options_value = 'bcgs bjacobi 1E-12 1E-10'
  []
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
  perf_graph = true
[]
