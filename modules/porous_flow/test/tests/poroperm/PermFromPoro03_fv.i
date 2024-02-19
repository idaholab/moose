# Testing permeability from porosity
# Trivial test, checking calculated permeability is correct
# k = k_anisotropic * B * exp(A * phi)

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 3
    xmin = 0
    xmax = 3
  []
[]

[GlobalParams]
  block = 0
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
    type = MooseVariableFVReal
    [FVInitialCondition]
      type = FVConstantIC
      value = 0
    []
  []
[]

[FVKernels]
  [flux]
    type = FVPorousFlowAdvectiveFlux
    gravity = '0 0 0'
    variable = pp
  []
[]

[FVBCs]
  [ptop]
    type = FVDirichletBC
    variable = pp
    boundary = right
    value = 0
  []
  [pbase]
    type = FVDirichletBC
    variable = pp
    boundary = left
    value = 1
  []
[]

[AuxVariables]
  [poro]
    type = MooseVariableFVReal
  []
  [perm_x]
    type = MooseVariableFVReal
  []
  [perm_y]
    type = MooseVariableFVReal
  []
  [perm_z]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [poro]
    type = ADPorousFlowPropertyAux
    property = porosity
    variable = poro
  []
  [perm_x]
    type = ADPorousFlowPropertyAux
    property = permeability
    variable = perm_x
    row = 0
    column = 0
  []
  [perm_y]
    type = ADPorousFlowPropertyAux
    property = permeability
    variable = perm_y
    row = 1
    column = 1
  []
  [perm_z]
    type = ADPorousFlowPropertyAux
    property = permeability
    variable = perm_z
    row = 2
    column = 2
  []
[]

[Postprocessors]
  [perm_x_bottom]
    type = PointValue
    variable = perm_x
    point = '0 0 0'
  []
  [perm_y_bottom]
    type = PointValue
    variable = perm_y
    point = '0 0 0'
  []
  [perm_z_bottom]
    type = PointValue
    variable = perm_z
    point = '0 0 0'
  []
  [perm_x_top]
    type = PointValue
    variable = perm_x
    point = '3 0 0'
  []
  [perm_y_top]
    type = PointValue
    variable = perm_y
    point = '3 0 0'
  []
  [perm_z_top]
    type = PointValue
    variable = perm_z
    point = '3 0 0'
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    # unimportant in this fully-saturated test
    m = 0.8
    alpha = 1e-4
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2.2e9
    viscosity = 1e-3
    density0 = 1000
    thermal_expansion = 0
  []
[]

[Materials]
  [permeability]
    type = ADPorousFlowPermeabilityExponential
    k_anisotropy = '1 0 0  0 2 0  0 0 0.1'
    poroperm_function = exp_k
    A = 10
    B = 1e-8
  []
  [temperature]
    type = ADPorousFlowTemperature
  []
  [massfrac]
    type = ADPorousFlowMassFraction
  []
  [eff_fluid_pressure]
    type = ADPorousFlowEffectiveFluidPressure
  []
  [ppss]
    type = ADPorousFlow1PhaseP
    porepressure = pp
    capillary_pressure = pc
  []
  [simple_fluid]
    type = ADPorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [porosity]
    type = ADPorousFlowPorosityConst
    porosity = 0.1
  []
  [relperm]
    type = ADPorousFlowRelativePermeabilityCorey
    n = 0 # unimportant in this fully-saturated situation
    phase = 0
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = Newton
  l_tol = 1E-5
  nl_abs_tol = 1E-3
  nl_rel_tol = 1E-8
  l_max_its = 200
  nl_max_its = 400
[]

[Outputs]
  file_base = 'PermFromPoro03_out'
  csv = true
  execute_on = 'timestep_end'
[]
