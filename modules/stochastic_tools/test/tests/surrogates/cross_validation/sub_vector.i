L = 1

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 100
    xmax = ${L}
    elem_type = EDGE3
  []
[]

[Variables]
  [T]
    order = SECOND
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = MatDiffusion
    variable = T
    diffusivity = k
  []
  [source]
    type = BodyForce
    variable = T
    value = 10000
  []
[]

[Materials]
  [conductivity]
    type = GenericConstantMaterial
    prop_names = k
    prop_values = 2.0
  []
[]

[BCs]
  [right]
    type = DirichletBC
    variable = T
    boundary = right
    value = 300
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[VectorPostprocessors]
  [T_vec]
    type = LineValueSampler
    variable = T
    start_point = '0 0 0'
    end_point = '${L} 0 0'
    num_points = 10
    sort_by = x
  []
[]
