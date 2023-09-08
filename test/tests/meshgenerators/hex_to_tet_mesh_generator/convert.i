[Mesh]
  [cube]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 3
    xmin = 0.0
    xmax = 1.0
    ymin = 0.0
    ymax = 1.0
    zmin = 0.0
    zmax = 1.0
  []
  [make_tet]
    type = HexToTetMeshGenerator
    input = cube
  []
  [add_sidesets]
    type = SideSetsAroundSubdomainGenerator
    input = make_tet
    block = 0
    new_boundary = '1'
  []
[]

[Variables]
  [u]
  []
[]

[Functions]
  [bc_func]
    type = ParsedFunction
    expression = 'exp(x)+exp(y)+z*z'
  []
  [forcing]
    type = ParsedFunction
    expression = '-(exp(x)+exp(y)+2)'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [forcing]
    type = BodyForce
    variable = u
    function = forcing
  []
[]

[BCs]
  [u]
    type = FunctionDirichletBC
    variable = u
    function = bc_func
    boundary = '1'
  []
[]

# Method of manufactured solutions obtains correct convergence rate, confirming that
# the produced mesh is acceptable.
[Postprocessors]
  [integral]
    type = ElementL2Error
    variable = u
    function = bc_func
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
