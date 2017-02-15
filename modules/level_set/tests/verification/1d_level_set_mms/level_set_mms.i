[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 32
  nx = 64
  uniform_refine = 0
[]

[Variables]
  [./phi]
  [../]
[]

[AuxVariables]
  [./v_x]
    initial_condition = 1
  [../]
[../]

[ICs]
  [./phi_ic]
    function = phi_exact
    variable = phi
    type = FunctionIC
  [../]
[]

[Functions]
  [./phi_exact]
    type = ParsedFunction
    value = 'a*exp(1/(10*t))*sin(2*pi*x/b) + 1'
    vars = 'a b'
    vals = '1 8'
  [../]
  [./phi_mms]
    type = ParsedFunction
    value = '-a*exp(1/(10*t))*sin(2*pi*x/b)/(10*t^2) + 2*pi*a*exp(1/(10*t))*cos(2*pi*x/b)/b'
    vars = 'a b'
    vals = '1 8'
  [../]
[]

[Kernels]
  [./phi_advection]
    type = LevelSetAdvection
    variable = phi
    velocity_x = v_x
  [../]
  [./phi_time]
    type = TimeDerivative
    variable = phi
  [../]
  [./phi_forcing]
    type = UserForcingFunction
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
    variable = phi
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
