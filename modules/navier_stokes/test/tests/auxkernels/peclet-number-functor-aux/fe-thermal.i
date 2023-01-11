rho = 1
mu = 1
k = 1
cp = 1

[GlobalParams]
  gravity = '0 0 0'
  pspg = true
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
  [corner_node]
    type = ExtraNodesetGenerator
    new_boundary = 'pinned_node'
    nodes = '0'
    input = gen
  []
[]

[AuxVariables]
  [Pe]
    family = MONOMIAL
    order = FIRST
  []
[]

[AuxKernels]
  [Pe]
    type = PecletNumberFunctorAux
    variable = Pe
    speed = speed
    thermal_diffusivity = 'thermal_diffusivity'
  []
[]

[Variables]
  [vel_x][]
  [vel_y][]
  [p][]
  [T][]
[]

[Kernels]
  # mass
  [mass]
    type = INSMass
    variable = p
    u = vel_x
    v = vel_y
    pressure = p
  []
  # x-momentum, space
  [x_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_x
    u = vel_x
    v = vel_y
    pressure = p
    component = 0
  []
  # y-momentum, space
  [y_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_y
    u = vel_x
    v = vel_y
    pressure = p
    component = 1
  []
  [temperature_space]
    type = INSTemperature
    variable = T
    u = vel_x
    v = vel_y
  []
[]

[BCs]
  [x_no_slip]
    type = DirichletBC
    variable = vel_x
    boundary = 'bottom right left'
    value = 0.0
  []
  [lid]
    type = FunctionDirichletBC
    variable = vel_x
    boundary = 'top'
    function = 'lid_function'
  []
  [y_no_slip]
    type = DirichletBC
    variable = vel_y
    boundary = 'bottom right top left'
    value = 0.0
  []
  [pressure_pin]
    type = DirichletBC
    variable = p
    boundary = 'pinned_node'
    value = 0
  []
  [T_hot]
    type = DirichletBC
    variable = T
    boundary = 'bottom'
    value = 1
  []
  [T_cold]
    type = DirichletBC
    variable = T
    boundary = 'top'
    value = 0
  []
[]

[Materials]
  [const]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'rho mu k cp'
    prop_values = '${rho} ${mu} ${k} ${cp}'
  []
  [speed]
    type = ADVectorMagnitudeFunctorMaterial
    x_functor = vel_x
    y_functor = vel_y
    vector_magnitude_name = speed
  []
  [thermal_diffusivity]
    type = ThermalDiffusivityFunctorMaterial
    k = ${k}
    rho = ${rho}
    cp = ${cp}
  []
[]

[Functions]
  [lid_function]
    # We pick a function that is exactly represented in the velocity
    # space so that the Dirichlet conditions are the same regardless
    # of the mesh spacing.
    type = ParsedFunction
    expression = '4*x*(1-x)'
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
    solve_type = 'NEWTON'
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type'
  petsc_options_value = 'asm      2               lu'
  line_search = 'none'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
