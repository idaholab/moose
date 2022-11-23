# Assign porosity and permeability variables from constant AuxVariables read from the mesh
# to create a heterogeneous model

[Mesh]
  type = FileMesh
  file = 'gold/constant_poroperm2_out.e'
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
    initial_from_file_var = poro
  []
  [permxx]
    family = MONOMIAL
    order = CONSTANT
    initial_from_file_var = permxx
  []
  [permxy]
    family = MONOMIAL
    order = CONSTANT
    initial_from_file_var = permxy
  []
  [permxz]
    family = MONOMIAL
    order = CONSTANT
    initial_from_file_var = permxz
  []
  [permyx]
    family = MONOMIAL
    order = CONSTANT
    initial_from_file_var = permyx
  []
  [permyy]
    family = MONOMIAL
    order = CONSTANT
    initial_from_file_var = permyy
  []
  [permyz]
    family = MONOMIAL
    order = CONSTANT
    initial_from_file_var = permyz
  []
  [permzx]
    family = MONOMIAL
    order = CONSTANT
    initial_from_file_var = permzx
  []
  [permzy]
    family = MONOMIAL
    order = CONSTANT
    initial_from_file_var = permzy
  []
  [permzz]
    family = MONOMIAL
    order = CONSTANT
    initial_from_file_var = permzz
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
  file_base = constant_poroperm2_out
[]
