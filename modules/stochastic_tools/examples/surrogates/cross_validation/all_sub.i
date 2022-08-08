[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmax = 0.03
  elem_type = EDGE3
[]

[Variables]
  [T]
    order = SECOND
    family = LAGRANGE
  []
[]

[Kernels]
  [diffusion]
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
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [max]
    type = NodalExtremeValue
    variable = T
    value_type = max
  []
[]

[Outputs]
[]
