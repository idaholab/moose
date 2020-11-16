[Mesh]
  [file]
    type = FileMeshGenerator
    file = nodal_normals_test_offset_nonmatching_gap.e
  []
  [./primary]
    input = file
    type = LowerDBlockFromSidesetGenerator
    sidesets = '2'
    new_block_id = '20'
  [../]
  [./secondary]
    input = primary
    type = LowerDBlockFromSidesetGenerator
    sidesets = '1'
    new_block_id = '10'
  [../]
  uniform_refine = 2
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [./T]
    block = '1 2'
  [../]
[]

[Kernels]
  [./conduction]
    type = Diffusion
    variable = T
    block = '1 2'
  [../]
  [./reaction]
    type = Reaction
    variable = T
    block = '1 2'
  [../]
[]

[Constraints]
  [./mortar]
    type = SpatiallyVaryingSource
    primary_boundary = 2
    secondary_boundary = 1
    primary_subdomain = 20
    secondary_subdomain = 10
    secondary_variable = T
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  solve_type = NEWTON
  type = Steady
[]

[Outputs]
  exodus = true
[]
