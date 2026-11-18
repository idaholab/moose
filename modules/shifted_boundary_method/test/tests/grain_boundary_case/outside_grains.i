[Problem]
  solve = false
[]

[Mesh]
  [boundary_mesh]
    type = FileMeshGenerator
    file = 'grain_boundary_only.msh'
  []
  [background_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
    nx = 4
    ny = 4
    subdomain_ids = 10
  []
  [grains]
    type = SubdomainGrainIDGenerator
    input = background_mesh
    boundary_mesh = boundary_mesh
  []
  final_generator = grains
[]

[AuxVariables]
  [element_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [element_id]
    type = ElementIDAux
    variable = element_id
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
