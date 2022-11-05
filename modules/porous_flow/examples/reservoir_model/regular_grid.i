# SPE 10 comparative problem - model 1
# Data and description from https://www.spe.org/web/csp/datasets/set01.htm
# Simple input file that just establishes gravity equilibrium in the model
#
# Heterogeneous permeability is included by reading data from an external file
# using the PiecewiseMultilinear function, and saving that data to an elemental
# AuxVariable that is then used in PorousFlowPermeabilityConstFromVar

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 20
  xmax = 762
  ymax = 15.24
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 -9.81 0'
  temperature_unit = Celsius
[]

[Variables]
  [porepressure]
    initial_condition = 20e6
  []
[]

[Functions]
  [perm_md_fcn]
    type = PiecewiseMultilinear
    data_file = spe10_case1.data
  []
[]

[BCs]
  [top]
    type = DirichletBC
    variable = porepressure
    value = 20e6
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
  [porosity]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 0.2
  []
  [perm_md]
    family = MONOMIAL
    order = CONSTANT
  []
  [perm]
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
  [perm_md]
    type = FunctionAux
    function = perm_md_fcn
    variable = perm_md
    execute_on = initial
  []
  [perm]
    type = ParsedAux
    variable = perm
    coupled_variables = perm_md
    expression = '9.869233e-16*perm_md'
    execute_on = initial
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
    perm_xx = perm
    perm_yy = perm
    perm_zz = perm
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
  end_time = 1e5
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-06
  steady_state_detection = true
  steady_state_tolerance = 1e-12
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e2
  []
[]

[Outputs]
  execute_on = 'initial timestep_end'
  exodus = true
  perf_graph = true
[]
