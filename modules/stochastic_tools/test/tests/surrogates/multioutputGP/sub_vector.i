L = 5.92999
subtract = 1.0
req1 = '${fparse L-subtract}'

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1000
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
    value = 71.4081 # 10333.1
  []
[]

[Materials]
  [conductivity]
    type = GenericConstantMaterial
    prop_names = k
    prop_values = 4.23908
  []
[]

[BCs]
  [right]
    type = DirichletBC
    variable = T
    boundary = right
    value = 465.688
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
    end_point = '${req1} 0 0'
    num_points = 2
    sort_by = x
  []
[]

# [Outputs]
#   csv = true
# []
