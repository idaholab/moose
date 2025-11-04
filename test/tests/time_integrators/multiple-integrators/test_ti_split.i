[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Problem]
  nl_sys_names = 'nl0 nl1'
[]

[Variables]
  [u]
    solver_sys = 'nl0'
  []
  [v]
    solver_sys = 'nl0'
  []
  [w]
    solver_sys = 'nl1'
  []
[]

[Kernels]
  [timeu]
    type = TimeDerivative
    variable = u
  []
  [timev]
    type = TimeDerivative
    variable = v
  []
  [timew]
    type = TimeDerivative
    variable = w
  []
  [diffu]
    type = Diffusion
    variable = u
  []
  [diffv]
    type = Diffusion
    variable = v
  []
  [diffw]
    type = Diffusion
    variable = w
  []
  [forceu]
    type = BodyForce
    variable = u
    function = force
  []
  [forcev]
    type = BodyForce
    variable = v
    function = force
  []
  [forcew]
    type = BodyForce
    variable = w
    function = force
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = 't^3*x*y'
  []
  [force]
    type = ParsedFunction
    expression = '3*x*y*t^2'
  []
[]

[BCs]
  [allu]
    type = FunctionDirichletBC
    variable = u
    function = exact
    boundary = 'left right top bottom'
  []
  [allv]
    type = FunctionDirichletBC
    variable = v
    function = exact
    boundary = 'left right top bottom'
  []
  [allw]
    type = FunctionDirichletBC
    variable = w
    function = exact
    boundary = 'left right top bottom'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'hypre'
  dt = 1
  end_time = 3
  [TimeIntegrators]
    [cn]
      type = CrankNicolson
      variables = 'u'
    []
    [ie]
      type = ImplicitEuler
      variables = 'v'
    []
    [cn2]
      type = CrankNicolson
      variables = 'w'
    []
  []
[]

[Postprocessors]
  [L2u]
    type = ElementL2FunctorError
    exact = exact
    approximate = u
  []
  [L2v]
    type = ElementL2FunctorError
    exact = exact
    approximate = v
  []
  [L2w]
    type = ElementL2FunctorError
    exact = exact
    approximate = w
  []
[]

[Outputs]
  csv = true
[]
