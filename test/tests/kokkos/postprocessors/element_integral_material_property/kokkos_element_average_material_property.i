[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 2
  ymax = 2
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = KokkosDiffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = KokkosDirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Materials]
  [mat]
    type = KokkosGenericConstantMaterial
    block = 0
    prop_names = prop
    prop_values = 2.0
  []
[]

[Postprocessors]
  [prop_average]
    type = KokkosElementAverageMaterialProperty
    mat_prop = prop
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
[]
