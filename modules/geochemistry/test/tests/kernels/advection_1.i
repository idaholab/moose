# A step-like initial concentration is advected to the right using a constant velocity.
# Because of the Dirichlet BC on the left, the step-like concentration profile is maintained (up to the usual numerical diffusion)
# Because upwinding_type=full in the ConservativeAdvection Kernel, there are no overshoots and undershoots
# The total amount of "conc" should increase by dt * velocity every timestep, as recorded by the front_position Postprocessor
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
[]

[Variables]
  [conc]
  []
[]

[ICs]
  [conc]
    type = FunctionIC
    function = 'if(x<=0.25, 1, 0)'
    variable = conc
  []
[]

[BCs]
  [left]
    type = DirichletBC
    boundary = left
    value = 1.0
    variable = conc
  []
[]

[Kernels]
  [dot]
    type = GeochemistryTimeDerivative
    variable = conc
  []
  [adv]
    type = ConservativeAdvection
    velocity = velocity
    upwinding_type = full
    variable = conc
  []
[]

[AuxVariables]
  [velocity]
    family = MONOMIAL_VEC
    order = CONSTANT
  []
[]

[AuxKernels]
  [velocity]
    type = VectorFunctionAux
    function = vel_fcn
    variable = velocity
  []
[]

[Functions]
  [vel_fcn]
    type = ParsedVectorFunction
    expression_x = 1
    expression_y = 0
    expression_z = 0
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 0.01
  end_time = 0.1
[]

[Postprocessors]
  [front_position]
    type = ElementIntegralVariablePostprocessor
    variable = conc
  []
[]

[Outputs]
  csv = true
[]

