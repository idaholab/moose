# This input file tests that the importer correctly detects a type mismatch
# between the exported property (Real) and the current simulation's property
# (RealVectorValue) for property named "diffusivity".

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
  []
[]

[Variables]
  [u]
    initial_condition = 0
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [time]
    type = TimeDerivative
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

[Materials]
  [stateful]
    type = VectorStatefulMaterial
  []
[]

[UserObjects]
  [importer]
    type = StatefulMaterialPropertyImporter
    file_base = 'stateful_export'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  num_steps = 1
  dt = 0.1
[]

[Outputs]
  exodus = false
[]
