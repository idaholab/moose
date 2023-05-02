[Mesh]
  [twoquad]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 4
    xmax = 2
  []
  [subdomain]
    type = SubdomainBoundingBoxGenerator
    input = twoquad
    block_id = 1
    bottom_left = '1 0 0'
    top_right = '2 1 0'
  []
  [sideset1]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain
    primary_block = 0
    paired_block = 1
    new_boundary = bar
  []
  [lowerblock1]
    type = LowerDBlockFromSidesetGenerator
    input = sideset1
    sidesets = bar
    new_block_id = 2
  []
  [sideset2]
    type = SideSetsBetweenSubdomainsGenerator
    input = lowerblock1
    primary_block = 1
    paired_block = 0
    new_boundary = baz
  []
  [delete]
    type = BlockDeletionGenerator
    input = sideset2
    block = 0
  []
  [lowerblock2]
    type = LowerDBlockFromSidesetGenerator
    input = delete
    sidesets = baz
    new_block_id = 3
  []
[]


[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./dt]
    type = TimeDerivative
    variable = u
  [../]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./top]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 10
  dt = 10

  solve_type = NEWTON

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
