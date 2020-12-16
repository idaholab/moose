[Mesh]
  [./mesh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 6
    ny = 2
    nz = 1
    elem_type = TET4
    xmin = -1
    xmax = 2
  [../]
  [./sideset]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y=x'
    input = mesh
    new_sideset_name = LD
  [../]
  [./sideset2]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y=-x+1'
    input = sideset
    new_sideset_name = LD2
  [../]
  [./lowerdim]
    type = MeshSideSetGenerator
    input = sideset2
    boundaries = 'LD LD2'
    block_id = 1
  [../]
[]

[Variables]
  [v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
[]

[FVKernels]
  [diff]
    type = FVLowerDimDiffusion
    variable = v
    coeff = coeff
    coeff_LD = 0.1
    thickness = 0.001
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = v
    boundary = left
    value = 7
  []
  [right]
    type = FVDirichletBC
    variable = v
    boundary = right
    value = 42
  []
[]

[Materials]
  [diff]
    type = ADGenericConstantMaterial
    prop_names = 'coeff'
    prop_values = '1'
  []
[]

[Problem]
  kernel_coverage_check = off
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
