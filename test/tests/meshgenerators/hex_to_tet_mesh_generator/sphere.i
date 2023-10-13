[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = permute_in.e
  []
  [make_tet]
    type = HexToTetMeshGenerator
    input = sphere
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
