[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 10.0
  nx = 100
[]

[Variables]
  [./phi]
  [../]
[]

[AuxVariables]
  [./vx]
  [../]

  [./force]
  [../]
[]

[ICs]
  [./vx]
    type = FunctionIC
    variable = vx
    function = vx_function
  [../]

  [./force]
    type = FunctionIC
    variable = force
    function = forcing
  [../]
[]

[Kernels]
  [./advection]
    type = MassConvectiveFlux
    variable = phi
    vel_x = vx
  [../]

  [./rhs]
    type = CoupledForce
    variable = phi
    v = force
  [../]
[]

[BCs]
  [./inflow_enthalpy]
    type = DirichletBC
    variable = phi
    boundary = 'left'
    value = 1
  [../]

  [./outflow_term]
    type = AdvectionBC
    variable = phi
    velocity_vector = 'vx'
    boundary = 'right'
  [../]
[]

[Functions]
  [./vx_function]
    type = ParsedFunction
    expression = '1 + x * x'
  [../]

  [./forcing]
    type = ParsedFunction
    expression = 'x'
  [../]

  [./analytical]
    type = ParsedFunction
    expression = '(1 + 0.5 * x * x) / (1 + x * x)'
  [../]
[]

[Postprocessors]
  [./error]
    type = ElementL2Error
    variable = phi
    function = analytical
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
