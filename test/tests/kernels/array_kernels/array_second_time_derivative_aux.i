# Second time derivatives of an array variable from coupledArrayDotDot(), compared with those of
# the same field on a standard variable from coupledDotDot(). Both components of the array
# variable u solve the heat equation of the standard variable w with Newmark-beta time
# integration, which also provides the second time derivatives, so those of all three are the
# same. The first time derivative differs from the second, so reading the wrong vector shows.
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 4
[]

[Variables]
  [u]
    components = 2
  []
  [w]
  []
[]

[ICs]
  [u]
    type = ArrayFunctionIC
    variable = u
    function = 'sin(pi*x) sin(pi*x)'
  []
  [w]
    type = FunctionIC
    variable = w
    function = 'sin(pi*x)'
  []
[]

[Kernels]
  [u_time]
    type = ArrayTimeDerivative
    variable = u
  []
  [u_diff]
    type = ArrayDiffusion
    variable = u
    diffusion_coefficient = dc
  []
  [w_time]
    type = TimeDerivative
    variable = w
  []
  [w_diff]
    type = Diffusion
    variable = w
  []
[]

[BCs]
  [u]
    type = ArrayDirichletBC
    variable = u
    boundary = 'left right'
    values = '0 0'
  []
  [w]
    type = DirichletBC
    variable = w
    boundary = 'left right'
    value = 0
  []
[]

[Materials]
  [dc]
    type = GenericConstantArray
    prop_name = dc
    prop_value = '1 1'
  []
[]

[AuxVariables]
  [u_dotdot]
    family = MONOMIAL
    order = CONSTANT
    components = 2
  []
  [w_dotdot]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [u_dotdot]
    type = ArraySecondTimeDerivativeAux
    variable = u_dotdot
    v = u
    execute_on = 'TIMESTEP_END'
  []
  [w_dotdot]
    type = SecondTimeDerivativeAux
    variable = w_dotdot
    v = w
    execute_on = 'TIMESTEP_END'
  []
[]

[Postprocessors]
  [u_dotdot_0]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u_dotdot
    component = 0
  []
  [u_dotdot_1]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u_dotdot
    component = 1
  []
  [w_dotdot]
    type = ElementIntegralVariablePostprocessor
    variable = w_dotdot
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  nl_rel_tol = 1e-12
  dt = 0.1
  num_steps = 3
  [TimeIntegrator]
    type = NewmarkBeta
  []
[]

[Outputs]
  csv = true
[]
