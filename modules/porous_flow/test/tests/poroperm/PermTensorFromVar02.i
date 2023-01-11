# Testing permeability calculated from scalar and tensor
# Trivial test, checking calculated permeability is correct
# when scalar is a FunctionAux.
# k = k_anisotropy * perm

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

[Functions]
  [perm_fn]
    type = ParsedFunction
    expression = '2*(x+1)'
  []
[]

[AuxVariables]
  [perm_var]
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
  [perm_var]
    type = FunctionAux
    function = perm_fn
    variable = perm_var
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
  [perm_x_left]
    type = PointValue
    variable = perm_x
    point = '0.5 0 0'
  []
  [perm_y_left]
    type = PointValue
    variable = perm_y
    point = '0.5 0 0'
  []
  [perm_z_left]
    type = PointValue
    variable = perm_z
    point = '0.5 0 0'
  []
  [perm_x_right]
    type = PointValue
    variable = perm_x
    point = '2.5 0 0'
  []
  [perm_y_right]
    type = PointValue
    variable = perm_y
    point = '2.5 0 0'
  []
  [perm_z_right]
    type = PointValue
    variable = perm_z
    point = '2.5 0 0'
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
  []
[]

[Materials]
  [permeability]
    type = PorousFlowPermeabilityTensorFromVar
    k_anisotropy = '1 0 0  0 2 0  0 0 0.1'
    perm = perm_var
  []
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
