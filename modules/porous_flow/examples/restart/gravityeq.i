# Initial run to establish gravity equilibrium. As only brine is present (no gas),
# we can use the single phase equation of state and kernels, reducing the computational
# cost. An estimate of the hydrostatic pressure gradient is used as the initial condition
# using an approximate brine density of 1060 kg/m^3.
# The end time is set to a large value (~100 years) to allow the pressure to reach
# equilibrium. Steady state detection is used to halt the run when a steady state is reached.

[Mesh]
  type = GeneratedMesh
  dim = 2
  ny = 10
  nx = 10
  ymax = 100
  xmax = 5000
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 -9.81 0'
  temperature_unit = Celsius
[]

[Variables]
  [porepressure]
  []
[]

[ICs]
  [porepressure]
    type = FunctionIC
    function = ppic
    variable = porepressure
  []
[]

[Functions]
  [ppic]
    type = ParsedFunction
    expression = '10e6 + 1060*9.81*(100-y)'
  []
[]

[BCs]
  [top]
    type = DirichletBC
    variable = porepressure
    value = 10e6
    boundary = top
  []
[]

[AuxVariables]
  [temperature]
    initial_condition = 50
  []
  [xnacl]
    initial_condition = 0.1
  []
  [brine_density]
    family = MONOMIAL
    order = CONSTANT
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

[AuxKernels]
  [brine_density]
    type = PorousFlowPropertyAux
    property = density
    variable = brine_density
    execute_on = 'initial timestep_end'
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
  [brine]
    type = BrineFluidProperties
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
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-13 0 0 0 1e-13 0  0 0 1e-13'
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
  end_time = 3e9
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-06
  steady_state_detection = true
  steady_state_tolerance = 1e-12
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e1
  []
[]

[Outputs]
  execute_on = 'initial timestep_end'
  exodus = true
  perf_graph = true
[]
