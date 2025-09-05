
[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 32
  nx = 64
[]

[Variables]
  [pf]
  []
[]

[AuxVariables]
  [velocity]
    family = LAGRANGE_VEC
  []
[]

[ICs]
  [pf_ic]
    function = pf_exact
    variable = pf
    type = FunctionIC
  []
  [vel_ic]
    type = VectorFunctionIC
    variable = velocity
    function = velocity_func
  []
[]

[Functions]
  [pf_exact]
    type = ParsedFunction
    expression = 'a*exp(1/(10*t))*sin(2*pi*x/b) + 1'
    symbol_names = 'a b'
    symbol_values = '1 8'
  []
  [pf_mms]
    type = ParsedFunction
    expression = '-a*exp(1/(10*t))*sin(2*pi*x/b)/(10*t^2) + 2*pi*a*exp(1/(10*t))*cos(2*pi*x/b)/b'
    symbol_names = 'a b'
    symbol_values = '1 8'
  []
  [velocity_func]
    type = ParsedVectorFunction
    expression_x = '1'
    expression_y = '1'
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = pf
  []
  [time_supg]
    type = ADPhaseFieldTimeDerivativeSUPG
    variable = pf
    velocity = velocity
  []
  [pf_advection]
    type = ADPhaseFieldAdvection
    variable = pf
    velocity = velocity
  []
  [pf_forcing]
    type = BodyForce
    variable = pf
    function = pf_mms
  []
  [pf_advection_supg]
    type = ADPhaseFieldAdvectionSUPG
    variable = pf
    velocity = velocity
  []
  [pf_forcing_supg]
    type = ADPhaseFieldForcingFunctionSUPG
    velocity = velocity
    variable = pf
    function = pf_mms
  []
[]

[Postprocessors]
  [error]
    type = ElementL2Error
    function = pf_exact
    variable = pf
  []
  [h]
    type = AverageElementSize
  []
  [point]
    type = PointValue
    point = '0.1 0 0'
    variable = pf
  []
[]

[Executioner]
  type = Transient
  start_time = 1
  dt = 0.01
  end_time = 1.25
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -sub_pc_type'
  petsc_options_value = 'asm      ilu'
  scheme = bdf2
  nl_rel_tol = 1e-12
[]

[Outputs]
  time_step_interval = 10
  execute_on = 'timestep_end'
  csv = true
[]
