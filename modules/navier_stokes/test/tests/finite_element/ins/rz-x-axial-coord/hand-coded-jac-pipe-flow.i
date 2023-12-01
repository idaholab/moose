mu=1
rho=1
pipe_length=10 # m
pipe_radius=1 # m
u_inlet=1

[GlobalParams]
  integrate_p_by_parts = false
  supg = true
  pspg = true
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${pipe_length}
    ymin = 0
    ymax = ${pipe_radius}
    nx = 50
    ny = 5
  []
  coord_type = 'RZ'
  rz_coord_axis = x
[]

[Variables]
  [velocity_x]
    family = LAGRANGE
  []
  [velocity_y]
    family = LAGRANGE
  []
  [p][]
[]

[Kernels]
  [mass]
    type = INSMassRZ
    variable = p
    u = velocity_x
    v = velocity_y
    pressure = p
  []
  [x_momentum]
    type = INSMomentumLaplaceFormRZ
    variable = velocity_x
    u = velocity_x
    v = velocity_y
    pressure = p
    component = 0
  []
  [y_momentum]
    type = INSMomentumLaplaceFormRZ
    variable = velocity_y
    u = velocity_x
    v = velocity_y
    pressure = p
    component = 1
  []
[]

[Functions]
  [vel_x_inlet]
    type = ParsedFunction
    expression = '${u_inlet} * (${pipe_radius}^2 - y^2)'
  []
[]

[BCs]
  [inlet_x]
    type = FunctionDirichletBC
    variable = velocity_x
    boundary = 'left'
    function = vel_x_inlet
  []
  [zero_y]
    type = FunctionDirichletBC
    variable = velocity_y
    boundary = 'left top bottom'
    function = 0
  []
  [zero_x]
    type = FunctionDirichletBC
    variable = velocity_x
    boundary = 'top'
    function = 0
  []
  # pressure is not integrated by parts so we cannot remove the nullspace through a natural condition
  [p_corner]
    type = DirichletBC
    boundary = 'right'
    value = 0
    variable = p
  []
[]

[Materials]
  [const]
    type = GenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '${rho} ${mu}'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
  line_search = 'none'
[]

[Outputs]
  exodus = true
[]
