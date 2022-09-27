[Mesh]
  [twoquad]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    xmax = 2
  []
  [subdomain]
    type = SubdomainBoundingBoxGenerator
    input = twoquad
    block_id = 1
    bottom_left = '1 0 0'
    top_right = '2 1 0'
  []
  [sideset]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain
    primary_block = 0
    paired_block = 1
    new_boundary = bar
  []
  [edge_elem]
    type = LowerDBlockFromSidesetGenerator
    input = sideset
    sidesets = bar
    new_block_id = 2
  []
  [delete]
    type = BlockDeletionGenerator
    input = edge_elem
    block = 0
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

