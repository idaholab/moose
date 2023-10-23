# Assign porosity and permeability variables from constant AuxVariables to create
# a heterogeneous model and solve with FV variables

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 3
    xmax = 3
    ymax = 3
    zmax = 3
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 -10'
[]

[Variables]
  [ppwater]
    type = MooseVariableFVReal
    initial_condition = 1.5e6
  []
[]

[AuxVariables]
  [poro]
    type = MooseVariableFVReal
  []
  [permxx]
    type = MooseVariableFVReal
  []
  [permxy]
    type = MooseVariableFVReal
  []
  [permxz]
    type = MooseVariableFVReal
  []
  [permyx]
    type = MooseVariableFVReal
  []
  [permyy]
    type = MooseVariableFVReal
  []
  [permyz]
    type = MooseVariableFVReal
  []
  [permzx]
    type = MooseVariableFVReal
  []
  [permzy]
    type = MooseVariableFVReal
  []
  [permzz]
    type = MooseVariableFVReal
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
    type = ADPorousFlowPropertyAux
    property = porosity
    variable = poromat
  []
  [permxxmat]
    type = ADPorousFlowPropertyAux
    property = permeability
    variable = permxxmat
    column = 0
    row = 0
  []
  [permxymat]
    type = ADPorousFlowPropertyAux
    property = permeability
    variable = permxymat
    column = 1
    row = 0
  []
  [permxzmat]
    type = ADPorousFlowPropertyAux
    property = permeability
    variable = permxzmat
    column = 2
    row = 0
  []
  [permyxmat]
    type = ADPorousFlowPropertyAux
    property = permeability
    variable = permyxmat
    column = 0
    row = 1
  []
  [permyymat]
    type = ADPorousFlowPropertyAux
    property = permeability
    variable = permyymat
    column = 1
    row = 1
  []
  [permyzmat]
    type = ADPorousFlowPropertyAux
    property = permeability
    variable = permyzmat
    column = 2
    row = 1
  []
  [permzxmat]
    type = ADPorousFlowPropertyAux
    property = permeability
    variable = permzxmat
    column = 0
    row = 2
  []
  [permzymat]
    type = ADPorousFlowPropertyAux
    property = permeability
    variable = permzymat
    column = 1
    row = 2
  []
  [permzzmat]
    type = ADPorousFlowPropertyAux
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
  [permx]
    type = FunctionIC
    function = permx
    variable = permxx
  []
  [permy]
    type = FunctionIC
    function = permy
    variable = permyy
  []
  [permz]
    type = FunctionIC
    function = permz
    variable = permzz
  []
[]

[Functions]
  [permx]
    type = ParsedFunction
    expression = '(1+x)*1e-11'
  []
  [permy]
    type = ParsedFunction
    expression = '(1+y)*1e-11'
  []
  [permz]
    type = ParsedFunction
    expression = '(1+z)*1e-11'
  []
[]

[FVKernels]
  [mass0]
    type = FVPorousFlowMassTimeDerivative
    variable = ppwater
  []
  [flux0]
    type = FVPorousFlowAdvectiveFlux
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
    type = ADPorousFlowTemperature
  []
  [ppss]
    type = ADPorousFlow1PhaseFullySaturated
    porepressure = ppwater
  []
  [massfrac]
    type = ADPorousFlowMassFraction
  []
  [simple_fluid]
    type = ADPorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [porosity]
    type = ADPorousFlowPorosityConst
    porosity = poro
  []
  [permeability]
    type = ADPorousFlowPermeabilityConstFromVar
    perm_xx = permxx
    perm_yy = permyy
    perm_zz = permzz
  []
  [relperm_water]
    type = ADPorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
[]

[Postprocessors]
  [mass_ph0]
    type = FVPorousFlowFluidMass
    fluid_component = 0
    execute_on = 'initial timestep_end'
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
  end_time = 100
  dt = 100
[]

[Outputs]
  execute_on = 'initial timestep_end'
  exodus = true
  perf_graph = true
[]
