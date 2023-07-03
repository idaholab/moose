mu=1
rho=1
pipe_length=10 # m
pipe_radius=1 # m
u_inlet=1

[GlobalParams]
  integrate_p_by_parts = true
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
[]

[Problem]
  coord_type = 'RZ'
  rz_coord_axis = x
[]

[Variables]
  [velocity]
    family = LAGRANGE_VEC
  []
  [p][]
[]

[Kernels]
  [mass]
    type = INSADMass
    variable = p
  []
  [mass_pspg]
    type = INSADMassPSPG
    variable = p
  []
  [momentum_convection]
    type = INSADMomentumAdvection
    variable = velocity
  []
  [momentum_viscous]
    type = INSADMomentumViscous
    variable = velocity
  []
  [momentum_pressure]
    type = INSADMomentumPressure
    variable = velocity
    pressure = p
  []
  [momentum_supg]
    type = INSADMomentumSUPG
    variable = velocity
    velocity = velocity
  []
[]

[Functions]
  [vel_x_inlet]
    type = ParsedFunction
    expression = '${u_inlet} * (${pipe_radius}^2 - y^2)'
  []
[]

[BCs]
  [inlet]
    type = VectorFunctionDirichletBC
    variable = velocity
    boundary = 'left'
    function_x = vel_x_inlet
    function_y = 0
  [../]
  [wall]
    type = VectorFunctionDirichletBC
    variable = velocity
    boundary = 'top'
    function_x = 0
    function_y = 0
  []
  [axis]
    type = ADVectorFunctionDirichletBC
    variable = velocity
    boundary = 'bottom'
    set_x_comp = false
    function_y = 0
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '${rho} ${mu}'
  []
  [ins_mat]
    type = INSADTauMaterial
    velocity = velocity
    pressure = p
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
