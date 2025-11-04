[Mesh]
  allow_renumbering = false
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 2
    ny = 2
  []
  [left]
    type = ParsedSubdomainMeshGenerator
    input = gen
    combinatorial_geometry = 'x < 0.5'
    block_id = 1
    block_name = LEFT_SIDE
  []
  [right]
    type = ParsedSubdomainMeshGenerator
    input = left
    combinatorial_geometry = 'x > 0.5'
    block_id = 2
    block_name = RIGHT_SIDE
  []
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Materials]
  [mat1]
    type = GenericFunctionMaterial
    prop_names = 'prop1 prop2 propt propx'
    prop_values = '1 2 t x'
    block = 1
  []
  [mat2]
    type = GenericFunctionMaterial
    prop_names = 'prop1 prop2 propt propx'
    prop_values = '10 20 t x'
    block = 2
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 1.0
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[VectorPostprocessors]
  [props]
    type = ElementMaterialSampler
    property = 'prop1 prop2 propt propx'
    elem_ids = '0 1'
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
[]
