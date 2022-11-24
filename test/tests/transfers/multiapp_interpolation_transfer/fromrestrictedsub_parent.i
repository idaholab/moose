[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  # The MultiAppGeometricInterpolationTransfer object only works with ReplicatedMesh
  parallel_type = replicated
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [elemental_from_sub]
    order = CONSTANT
    family = MONOMIAL
  []
  [nodal_from_sub]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '0.05 0.5 0 0.55 0.5 0'
    input_files = fromrestrictedsub_sub.i
    output_in_position = true
  []
[]

[Transfers]
  [elemental_fromsub]
    type = MultiAppGeometricInterpolationTransfer
    from_multi_app = sub
    source_variable = elemental
    variable = elemental_from_sub
  []
  [nodal_fromsub]
    type = MultiAppGeometricInterpolationTransfer
    from_multi_app = sub
    source_variable = nodal
    variable = nodal_from_sub
  []
[]
