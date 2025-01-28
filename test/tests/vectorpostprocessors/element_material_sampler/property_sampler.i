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
    prop_names = 'prop1 prop2 propt'
    prop_values = '1 2 t'
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
    property = 'prop1 prop2 propt'
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
[]
