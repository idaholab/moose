# Testing permeability from porosity
# Trivial test, checking calculated permeability is correct
# k = k_anisotropic * k
# with log k = A * phi + B

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 3
  xmin = 0
  xmax = 3
[]

[GlobalParams]
  block = 0
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
    [InitialCondition]
      type = ConstantIC
      value = 0
    []
  []
[]

[Kernels]
  [flux]
    type = PorousFlowAdvectiveFlux
    gravity = '0 0 0'
    variable = pp
  []
[]

[BCs]
  [ptop]
    type = DirichletBC
    variable = pp
    boundary = right
    value = 0
  []
  [pbase]
    type = DirichletBC
    variable = pp
    boundary = left
    value = 1
  []
[]

[AuxVariables]
  [poro]
    order = CONSTANT
    family = MONOMIAL
  []
  [perm_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [perm_y]
    order = CONSTANT
    family = MONOMIAL
  []
  [perm_z]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [poro]
    type = PorousFlowPropertyAux
    property = porosity
    variable = poro
  []
  [perm_x]
    type = PorousFlowPropertyAux
    property = permeability
    variable = perm_x
    row = 0
    column = 0
  []
  [perm_y]
    type = PorousFlowPropertyAux
    property = permeability
    variable = perm_y
    row = 1
    column = 1
  []
  [perm_z]
    type = PorousFlowPropertyAux
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
  [temperature]
    type = PorousFlowTemperature
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [eff_fluid_pressure]
    type = PorousFlowEffectiveFluidPressure
  []
  [ppss]
    type = PorousFlow1PhaseP
    porepressure = pp
    capillary_pressure = pc
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosity
    porosity_zero = 0.1
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 0 # unimportant in this fully-saturated situation
    phase = 0
  []

  [permeability]
    type = PorousFlowPermeabilityExponential
    k_anisotropy = '1 0 0  0 2 0  0 0 0.1'
    poroperm_function = log_k
    A = 4.342945
    B = -8
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
  []
[]

[Executioner]
  solve_type = Newton
  type = Steady
  l_tol = 1E-5
  nl_abs_tol = 1E-3
  nl_rel_tol = 1E-8
  l_max_its = 200
  nl_max_its = 400

  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -ksp_type -ksp_gmres_restart'
  petsc_options_value = ' asm      2              lu            gmres     200'
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
