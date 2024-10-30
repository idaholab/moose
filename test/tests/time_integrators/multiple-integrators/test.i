[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u][]
  [v][]
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
  [diffu]
    type = Diffusion
    variable = u
  []
  [diffv]
    type = Diffusion
    variable = v
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
  []
[]

[Postprocessors]
  [L2u]
    type = ElementL2Error
    function = exact
    variable = u
  []
  [L2v]
    type = ElementL2Error
    function = exact
    variable = v
  []
[]

[Outputs]
  csv = true
[]
