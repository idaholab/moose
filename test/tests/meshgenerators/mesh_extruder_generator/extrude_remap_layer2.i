[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = multiblock.e
  []

  [./extrude]
    type = MeshExtruderGenerator
    input = fmg
    num_layers = 6
    extrusion_vector = '0 0 2'
    bottom_sideset = 'new_bottom'
    top_sideset = 'new_top'

    # Remap layers
    existing_subdomains = '1 2 5'
    layers = '1 3 5'
    new_ids = '10 12 15' # Repeat this remapping for each layer
  []
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = 'new_bottom'
    value = 0
  [../]

  [./top]
    type = DirichletBC
    variable = u
    boundary = 'new_top'
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
