# Field model generated using geophysical modelling tool

[Mesh]
  type = FileMesh
  file = field.e
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 -9.81'
  temperature_unit = Celsius
[]

[Variables]
  [porepressure]
    initial_condition = 20e6
  []
[]

[AuxVariables]
  [temperature]
    initial_condition = 50
  []
  [xnacl]
    initial_condition = 0.1
  []
  [porosity]
    family = MONOMIAL
    order = CONSTANT
    initial_from_file_var = poro
  []
  [permx_md]
    family = MONOMIAL
    order = CONSTANT
    initial_from_file_var = permX
  []
  [permy_md]
    family = MONOMIAL
    order = CONSTANT
    initial_from_file_var = permY
  []
  [permz_md]
    family = MONOMIAL
    order = CONSTANT
    initial_from_file_var = permZ
  []
  [permx]
    family = MONOMIAL
    order = CONSTANT
  []
  [permy]
    family = MONOMIAL
    order = CONSTANT
  []
  [permz]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [permx]
    type = ParsedAux
    variable = permx
    coupled_variables = permx_md
    expression = '9.869233e-16*permx_md'
    execute_on = initial
  []
  [permy]
    type = ParsedAux
    variable = permy
    coupled_variables = permy_md
    expression = '9.869233e-16*permy_md'
    execute_on = initial
  []
  [permz]
    type = ParsedAux
    variable = permz
    coupled_variables = permz_md
    expression = '9.869233e-16*permz_md'
    execute_on = initial
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    variable = porepressure
  []
  [flux0]
    type = PorousFlowFullySaturatedDarcyFlow
    variable = porepressure
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = porepressure
    number_fluid_phases = 1
    number_fluid_components = 1
  []
[]

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
  [watertab]
    type = TabulatedFluidProperties
    fp = water
    save_file = false
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temperature
  []
  [ps]
    type = PorousFlow1PhaseFullySaturated
    porepressure = porepressure
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [brine]
    type = PorousFlowBrine
    compute_enthalpy = false
    compute_internal_energy = false
    xnacl = xnacl
    phase = 0
    water_fp = watertab
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = porosity
  []
  [permeability]
    type = PorousFlowPermeabilityConstFromVar
    perm_xx = permx
    perm_yy = permy
    perm_zz = permz
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
  dt = 1e2
  end_time = 1e2
[]

[Outputs]
  execute_on = 'initial timestep_end'
  exodus = true
  perf_graph = true
[]
