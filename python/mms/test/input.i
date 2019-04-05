[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 8
[]

[Variables]
  [u][]
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [force]
    type = BodyForce
    variable = u
    function = force
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    value = 'a*sin(2*pi*a*x*y)'
    vars = 'a'
    vals = '2'
  []
  [force]
    type = ParsedFunction
    value = '4*x^2*pi^2*a^3*sin(2*x*y*pi*a) + 4*y^2*pi^2*a^3*sin(2*x*y*pi*a)'
    vars = 'a'
    vals = '2'
  []
[]

[BCs]
  [all]
    type = FunctionDirichletBC
    variable = u
    function = exact
    boundary = 'left right top bottom'
  []
[]

[Postprocessors]
  [error]
    type = ElementL2Error
    function = exact
    variable = u
  []
  [h]
    type = AverageElementSize
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Adaptivity]
  marker = marker
  [Markers/marker]
    type = UniformMarker
    mark = refine
  []
[]

[Outputs]
  csv = true
[]
