[Problem]
  kernel_coverage_check = FALSE
[]

[GlobalParams]
  block = '0 1 3'
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 2
    xmin = 0
    xmax = 3
    ymin = 0
    ymax = 2
  []
  [block_left]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 0
    block_name = material_left
    bottom_left = '0 0 0'
    top_right = '1.0 2.0 0'
  []
  [block_right]
    type = SubdomainBoundingBoxGenerator
    input = block_left
    block_id = 1
    block_name = material_right
    bottom_left = '2.0 0 0'
    top_right = '3.0 2.0 0'
  []
  [block_middle]
    type = SubdomainBoundingBoxGenerator
    input = block_right
    block_id = 2
    block_name = material_null
    bottom_left = '1.0 0 0'
    top_right = '2.0 1.0 0'
  []
  [block_middle_new]
    type = SubdomainBoundingBoxGenerator
    input = block_middle
    block_id = 3
    block_name = material_middle
    bottom_left = '1.0 1.0 0'
    top_right = '2.0 2.0 0'
  []
  use_displaced_mesh = false
[]

[Variables]
  [diff]
    order = FIRST
  []
[]

[Kernels]
  [diffusion]
    type = MatDiffusion
    variable = diff
    diffusivity = 'k'
  []
[]

[Materials]
  [material_left_cond]
    block = 0
    type = GenericConstantMaterial
    prop_names = 'k'
    prop_values = 26.0
  []
  [material_right_cond]
    block = 1
    type = GenericConstantMaterial
    prop_names = 'k'
    prop_values = 35.0
  []
  [material_middle_cond]
    block = 3
    type = GenericConstantMaterial
    prop_names = 'k'
    prop_values = 10.0
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = diff
    boundary = left
    value = 10
  []

  [right]
    type = DirichletBC
    variable = diff
    boundary = right
    value = 0
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  dt = 1
  end_time = 1
[]

[Postprocessors]
  [T3]
    type = ElementAverageValue
    variable = diff
    block = '3'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
