# Checking that gravity head is established in the steady-state situation when 0<saturation<1 (note the strictly less-than).
# 2phase (PP), 2components, vanGenuchten, constant fluid bulk-moduli for each phase, constant viscosity, constant permeability, Corey relative perm
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
  [ppwater]
    initial_condition = -1.0
  []
  [ppgas]
    initial_condition = 0
  []
[]

[AuxVariables]
  [massfrac_ph0_sp0]
    initial_condition = 1
  []
  [massfrac_ph1_sp0]
    initial_condition = 0
  []
[]

[Kernels]
  [flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = ppwater
    gravity = '-1 0 0'
  []
  [flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = ppgas
    gravity = '-1 0 0'
  []
[]

[BCs]
  [ppwater]
    type = DirichletBC
    boundary = right
    variable = ppwater
    value = -1
  []
  [ppgas]
    type = DirichletBC
    boundary = right
    variable = ppgas
    value = 0
  []
[]

[Functions]
  [ana_ppwater]
    type = ParsedFunction
    symbol_names = 'g B p0 rho0'
    symbol_values = '1 2 pp_water_top 1'
    expression = '-B*log(exp(-p0/B)+g*rho0*x/B)' # expected pp at base
  []
  [ana_ppgas]
    type = ParsedFunction
    symbol_names = 'g B p0 rho0'
    symbol_values = '1 1 pp_gas_top 0.1'
    expression = '-B*log(exp(-p0/B)+g*rho0*x/B)' # expected pp at base
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'ppwater ppgas'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1
  []
[]

[FluidProperties]
  [simple_fluid0]
    type = SimpleFluidProperties
    bulk_modulus = 2
    density0 = 1
    viscosity = 1
    thermal_expansion = 0
  []
  [simple_fluid1]
    type = SimpleFluidProperties
    bulk_modulus = 1
    density0 = 0.1
    viscosity = 0.5
    thermal_expansion = 0
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow2PhasePP
    phase0_porepressure = ppwater
    phase1_porepressure = ppgas
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph1_sp0'
  []
  [simple_fluid0]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid0
    phase = 0
  []
  [simple_fluid1]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid1
    phase = 1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0  0 2 0  0 0 3'
  []
  [relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    n = 1
    phase = 0
  []
  [relperm_gas]
    type = PorousFlowRelativePermeabilityCorey
    n = 1
    phase = 1
  []
[]

[Postprocessors]
  [pp_water_top]
    type = PointValue
    variable = ppwater
    point = '0 0 0'
  []
  [pp_water_base]
    type = PointValue
    variable = ppwater
    point = '-1 0 0'
  []
  [pp_water_analytical]
    type = FunctionValuePostprocessor
    function = ana_ppwater
    point = '-1 0 0'
  []
  [pp_gas_top]
    type = PointValue
    variable = ppgas
    point = '0 0 0'
  []
  [pp_gas_base]
    type = PointValue
    variable = ppgas
    point = '-1 0 0'
  []
  [pp_gas_analytical]
    type = FunctionValuePostprocessor
    function = ana_ppgas
    point = '-1 0 0'
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
  []
[]


[Executioner]
  type = Steady
  solve_type = Newton
[]

[Outputs]
  file_base = grav02b
  [csv]
    type = CSV
  []
  exodus = false
[]
