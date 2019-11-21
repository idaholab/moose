[Mesh]
  # set this to false to prevent element renumbering in distributed mesh
  allow_renumbering = false
  [gmg]
    type = MetaDataGenerator
    dim = 2
    nx = 12
    ny = 10
    xmin = 4
    xmax = 7
    mod = 4
  []
[]

[Variables]
  [u][]
[]

[AuxVariables]
  [v]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
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

[AuxKernels]
  [aux]
    type = MeshMetaDataAux
    variable = v
    mesh_generator_name = gmg
    mesh_meta_data = 'modded_element_id'
  []
[]

[Executioner]
  type = Transient
  num_steps = 20
  dt = 1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  csv = true
[]
