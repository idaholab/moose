[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    subdomain_ids = '1 1 1 1 1
                     1 1 1 1 1
                     1 1 1 1 1
                     1 1 1 1 1
                     1 1 1 1 1
                     '
  []

  [gmg2]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    subdomain_ids = '2 2 2 2 2
                     2 2 2 2 2
                     2 2 2 2 2
                     2 2 2 2 2
                     2 2 2 2 2
                     '
    # The following triggers generation of new common boundary ids in
    # PatternedMeshGenerator
    boundary_id_offset = 1
  []

  [pmg]
    type = PatternedMeshGenerator
    inputs = 'gmg gmg2'
    pattern = '1 0 ;
               0 1'
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = MatCoefDiffusion
    variable = u
    conductivity = conductivity
  []
[]

[BCs]
  [top]
    type = DirichletBC
    variable = u
    boundary = 'top'
    value = 1
  []
  [bottom]
    type = DirichletBC
    variable = u
    boundary = 'bottom'
    value = 0
  []
[]

[Materials]
  [mat1]
    type = GenericConstantMaterial
    block = 1
    prop_names = conductivity
    prop_values = 100
  []
  [mat2]
    type = GenericConstantMaterial
    block = 2
    prop_names = conductivity
    prop_values = 1e-4
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
