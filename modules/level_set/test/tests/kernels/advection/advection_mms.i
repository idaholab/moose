[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 12
  nx = 48
[]

[Adaptivity]
  steps = 5
  marker = marker
  [./Markers]
    [./marker]
      type = UniformMarker
      mark = REFINE
    [../]
  [../]
[]

[Variables]
  [./phi]
  [../]
[]

[AuxVariables]
  [./velocity]
    family = LAGRANGE_VEC
  [../]
[]

[ICs]
  [./vel_ic]
    type = VectorFunctionIC
    variable = velocity
    function = velocity_func
  []
[]

[BCs]
  [./left]
    type = FunctionDirichletBC
    boundary = 'left'
    function = phi_exact
    variable = phi
  [../]
[]

[Functions]
  [./phi_exact]
    type = ParsedFunction
    expression = 'a*sin(pi*x/b)*cos(pi*x)'
    symbol_names = 'a b'
    symbol_values = '2 12'
  [../]
  [./phi_mms]
    type = ParsedFunction
    expression = '-2*pi*a*sin(pi*x)*sin(pi*x/b) + 2*pi*a*cos(pi*x)*cos(pi*x/b)/b'
    symbol_names = 'a b'
    symbol_values = '2 12'
  [../]
  [./velocity_func]
    type = ParsedVectorFunction
    expression_x = '2'
    expression_y = '2'
  [../]
[]

[Kernels]
  [./phi_advection]
    type = LevelSetAdvection
    variable = phi
    velocity = velocity
  [../]
  [./phi_forcing]
    type = BodyForce
    variable = phi
    function = phi_mms
  [../]
[]

[Postprocessors]
  [./error]
    type = ElementL2Error
    function = phi_exact
    variable = phi
  [../]
  [./h]
    type = AverageElementSize
  [../]
[]

[VectorPostprocessors]
  active = ''
  [./results]
    type = LineValueSampler
    variable = phi
    start_point = '0 0 0'
    end_point = '12 0 0'
    num_points = 500
    sort_by = x
  [../]
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-10
  solve_type = NEWTON
  # A steady-state pure advection problem is numerically challenging,
  # it has a zero diagonal in the Jabocian matrix. The following solver
  # settings seem to reliably solve this problem.
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
[]

[Outputs]
  execute_on = 'TIMESTEP_END'
  csv = true
[]
