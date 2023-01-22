[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 32
  nx = 64
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
  [./phi_ic]
    function = phi_exact
    variable = phi
    type = FunctionIC
  [../]
  [./vel_ic]
    type = VectorFunctionIC
    variable = velocity
    function = velocity_func
  []
[]

[Functions]
  [./phi_exact]
    type = ParsedFunction
    expression = 'a*exp(1/(10*t))*sin(2*pi*x/b) + 1'
    symbol_names = 'a b'
    symbol_values = '1 8'
  [../]
  [./phi_mms]
    type = ParsedFunction
    expression = '-a*exp(1/(10*t))*sin(2*pi*x/b)/(10*t^2) + 2*pi*a*exp(1/(10*t))*cos(2*pi*x/b)/b'
    symbol_names = 'a b'
    symbol_values = '1 8'
  [../]
  [./velocity_func]
    type = ParsedVectorFunction
    expression_x = '1'
    expression_y = '1'
  [../]
[]

[Kernels]
  [./time]
    type = TimeDerivative
    variable = phi
  [../]
  [./time_supg]
    type = LevelSetTimeDerivativeSUPG
    variable = phi
    velocity = velocity
  [../]
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
  [./phi_advection_supg]
    type = LevelSetAdvectionSUPG
    variable = phi
    velocity = velocity
  [../]
  [./phi_forcing_supg]
    type = LevelSetForcingFunctionSUPG
    velocity = velocity
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
  [./point]
    type = PointValue
    point = '0.1 0 0'
    variable = phi
  [../]
[]

[Executioner]
  type = Transient
  start_time = 1
  dt = 0.01
  end_time = 1.25
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_sub_type'
  petsc_options_value = 'asm      ilu'
  scheme = bdf2
  nl_rel_tol = 1e-12
[]

[Outputs]
  interval = 10
  execute_on = 'timestep_end'
  csv = true
[]
